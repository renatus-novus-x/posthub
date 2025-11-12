/* posthub.c - Minimal 8.3-safe Maildir-like messenger (Windows + POSIX/elf2x68k)
 * Build (Windows, MSVC):  cl /O2 /Fe:posthub.exe posthub.c
 * Build (MinGW):          gcc -O2 -o posthub.exe posthub.c
 * Build (macOS/Linux):    cc -O2 -o posthub posthub.c
 * Build (elf2x68k):       gcc -O2 -o posthub.x posthub.c
 *
 * Usage:
 *   posthub.x send all "hello!"
 *   posthub.x send <user> "message"
 *   posthub.x recv <user>
 *
 * Layout (must exist beforehand):
 *   ROOT/ (default "./POSTHUB" or env POSTHUB_ROOT)
 *     users.txt
 *     <user>/Maildir/tmp
 *                   /new
 *                   /cur
 *
 * Design notes:
 *   - 8.3 filenames only: XXXXXXXX.MSG (uppercase hex), to be safe for Human68k.
 *   - Read/unread is managed by directory transition: new/<name>.MSG -> cur/<name>.MSG.
 *   - Atomic delivery: write to tmp/NAME.MSG then rename() to new/NAME.MSG.
 *   - Directory enumeration: WinAPI (FindFirstFile) on Windows, dirent on others.
 *   - File flush boundary: fflush + fsync/_commit (best effort on each platform).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <io.h>
  #include <process.h>
  #define FILESEP '\\'
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <errno.h>
  #define FILESEP '/'
#endif

#ifdef _MSC_VER
  #define snprintf _snprintf
#endif

#define PATHLEN 1024
#define LINELEN 4096
#define IDLEN     64

/* ---------- flush helper (fsync equivalent) ---------- */
#if defined(_WIN32)
  #define FLUSH_FILE(f)  _commit(_fileno(f))
#elif defined(__unix__) || defined(__APPLE__)
  #define FLUSH_FILE(f)  fsync(fileno(f))
#else
  #define FLUSH_FILE(f)  0
#endif

/* ---------- small portability helpers ---------- */

/* simple toupper for ASCII */
static char upc(char c) {
  return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

/* case-insensitive equals for ASCII extensions like ".MSG" */
static int eq_icase(const char* a, const char* b) {
  while (*a && *b) {
    if (upc(*a) != upc(*b)) return 0;
    ++a; ++b;
  }
  return *a == '\0' && *b == '\0';
}

static const char* get_root(void) {
  const char* e = getenv("POSTHUB_ROOT");
  return (e && *e) ? e : "./POSTHUB";
}

/* join a/b/c using platform separator */
static void path_join3(char* out, size_t cap, const char* a, const char* b, const char* c) {
  size_t n = 0;
  #define APPEND(S) do { \
    if ((S) && *(S)) { \
      size_t L = strlen(S); \
      if (n && out[n-1] != FILESEP) { if (n+1 < cap) out[n++] = FILESEP; } \
      if (n+L < cap) { memcpy(out+n, (S), L); n += L; } \
    } \
  } while(0)
  if (cap == 0) return;
  out[0] = '\0';
  APPEND(a); APPEND(b); APPEND(c);
  if (n < cap) out[n] = '\0';
  #undef APPEND
}

static void path_join2(char* out, size_t cap, const char* a, const char* b) {
  path_join3(out, cap, a, b, NULL);
}

#if defined(_WIN32)
static unsigned long get_pidlike(void) { return (unsigned long)GetCurrentProcessId(); }
#else
static unsigned long get_pidlike(void) { return (unsigned long)getpid(); }
#endif

/* ---------- 8.3 unique filename generator ---------- */
/* Pattern: XXXXXXXX.MSG (uppercase hex)
 * name = tttttt p c  (6 hex from epoch seconds L24 + 1 hex pid nibble + 1 hex counter)
 */
static int file_exists(const char* path) {
  FILE* f = fopen(path, "rb");
  if (f) { fclose(f); return 1; }
  return 0;
}

static void mk_unique_file_83(char* out, size_t cap, const char* dir) {
  static unsigned counter = 0;
  unsigned long ep = (unsigned long)time(NULL);
  unsigned long t24 = ep & 0xFFFFFFul;            /* 6 hex */
  unsigned long p   = (get_pidlike() & 0xF);      /* 1 hex nibble */
  unsigned long c   = (counter++) & 0xF;          /* 1 hex nibble */
  char name[16];
#if defined(_WIN32)
  snprintf(name, sizeof(name), "%06lX%1lX%1lX", t24, p, c);
  snprintf(out, cap, "%s\\%s.MSG", dir, name);
#else
  snprintf(name, sizeof(name), "%06lX%1lX%1lX", t24, p, c);
  snprintf(out, cap, "%s/%s.MSG", dir, name);
#endif
  /* probe and advance counter on collision */
  while (file_exists(out)) {
    c = (counter++) & 0xF;
#if defined(_WIN32)
    snprintf(name, sizeof(name), "%06lX%1lX%1lX", t24, p, c);
    snprintf(out, cap, "%s\\%s.MSG", dir, name);
#else
    snprintf(name, sizeof(name), "%06lX%1lX%1lX", t24, p, c);
    snprintf(out, cap, "%s/%s.MSG", dir, name);
#endif
  }
}

/* ---------- IO helpers ---------- */

static int write_full(const char* path, const void* data, size_t len) {
  FILE* f = fopen(path, "wb");
  if (!f) return -1;
  if (len && fwrite(data, 1, len, f) != len) { fclose(f); return -1; }
  if (fflush(f) != 0) { fclose(f); return -1; }
  if (FLUSH_FILE(f) != 0) { fclose(f); return -1; }
  if (fclose(f) != 0) return -1;
  return 0;
}

/* tmp/NAME.MSG -> new/NAME.MSG by atomic rename */
static int deliver_into_new(const char* mdir_user, const char* payload, size_t plen,
                            char* out_name, size_t out_cap)
{
  char tmpdir[PATHLEN], newdir[PATHLEN];
  char tmppath[PATHLEN], newpath[PATHLEN];
  const char* base;

  path_join3(tmpdir, sizeof(tmpdir), mdir_user, "tmp", NULL);
  path_join3(newdir, sizeof(newdir), mdir_user, "new", NULL);
  mk_unique_file_83(tmppath, sizeof(tmppath), tmpdir);

  /* newpath = new/<basename> */
  {
    const char* p = strrchr(tmppath, FILESEP);
    if (!p) return -1;
    path_join3(newpath, sizeof(newpath), newdir, p+1, NULL);
  }

  if (write_full(tmppath, payload, plen) != 0) return -1;
  if (rename(tmppath, newpath) != 0) { remove(tmppath); return -1; }

  base = strrchr(newpath, FILESEP);
  base = base ? base + 1 : newpath;
  snprintf(out_name, out_cap, "%s", base);
  return 0;
}

/* ---------- send path ---------- */

static int send_to_user(const char* root, const char* user, const char* msg) {
  char mdir_user[PATHLEN], maildir[PATHLEN], name[PATHLEN];
  path_join3(mdir_user, sizeof(mdir_user), root, user, "Maildir");
  /* deliver to Maildir/new via tmp->rename */
  if (deliver_into_new(mdir_user, msg, strlen(msg), name, sizeof(name)) != 0) {
    fprintf(stderr, "[send] write/rename failed: user=%s\n", user);
    return -1;
  }
  (void)maildir;
  return 0;
}

static int send_to_all(const char* root, const char* msg) {
  char upath[PATHLEN];
  FILE* u;
  path_join2(upath, sizeof(upath), root, "users.txt");
  u = fopen(upath, "rb");
  if (!u) { fprintf(stderr, "open users.txt failed: %s\n", upath); return -1; }

  {
    char line[IDLEN]; int ok = 0;
    while (fgets(line, sizeof(line), u)) {
      size_t n = strlen(line);
      while (n && (line[n-1]=='\r' || line[n-1]=='\n')) line[--n]='\0';
      if (!line[0]) continue;
      if (send_to_user(root, line, msg) == 0) ok++;
    }
    fclose(u);
    if (ok == 0) { fprintf(stderr, "[send all] delivered=0 (check users.txt)\n"); return -1; }
  }
  return 0;
}

/* ---------- recv path ---------- */

static int has_msg_ext(const char* s) {
  const char* p = strrchr(s, '.');
  if (!p) return 0;
  return eq_icase(p, ".MSG");
}

/* print entire file, then move it to cur/ with the same name */
static int consume_and_move(const char* mdir_user, const char* name) {
  char newfile[PATHLEN], curfile[PATHLEN];
  FILE* f;
  path_join3(newfile, sizeof(newfile), mdir_user, "new", name);

  f = fopen(newfile, "rb");
  if (!f) return 0; /* not ready or vanished */
  {
    char buf[4096];
    size_t r, printed = 0;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
      fwrite(buf, 1, r, stdout);
      printed += r;
    }
    if (printed == 0 || (printed > 0 && buf[r? r-1 : 0] != '\n')) fputc('\n', stdout);
  }
  fclose(f);

  path_join3(curfile, sizeof(curfile), mdir_user, "cur", name);
  if (rename(newfile, curfile) != 0) {
    /* keep it in new/ for retry */
    return 0;
  }
  return 1;
}

#if defined(_WIN32)
/* Windows: enumerate new\*.MSG via FindFirstFile */
static int recv_for_user(const char* root, const char* user) {
  char mdir_user[PATHLEN], pat[PATHLEN];
  int processed = 0;
  WIN32_FIND_DATAA fd;
  HANDLE h;

  path_join3(mdir_user, sizeof(mdir_user), root, user, "Maildir");
  {
    char newdir[PATHLEN];
    path_join3(newdir, sizeof(newdir), mdir_user, "new", NULL);
    snprintf(pat, sizeof(pat), "%s%c%s", newdir, FILESEP, "*.MSG");
  }

  h = FindFirstFileA(pat, &fd);
  if (h == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "(%s) no new messages or cannot open directory\n", user);
    return 0;
  }
  do {
    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      if (has_msg_ext(fd.cFileName)) {
        if (consume_and_move(mdir_user, fd.cFileName)) processed++;
      }
    }
  } while (FindNextFileA(h, &fd));
  FindClose(h);

  fprintf(stderr, "(%s) received: %d\n", user, processed);
  return processed;
}
#else
/* POSIX/elf2x68k: enumerate via dirent */
static int recv_for_user(const char* root, const char* user) {
  char mdir_user[PATHLEN], newdir[PATHLEN];
  DIR* d; struct dirent* e;
  int processed = 0;

  path_join3(mdir_user, sizeof(mdir_user), root, user, "Maildir");
  path_join3(newdir, sizeof(newdir), mdir_user, "new", NULL);
  d = opendir(newdir);
  if (!d) {
    fprintf(stderr, "(%s) cannot open %s\n", user, newdir);
    return 0;
  }
  while ((e = readdir(d)) != NULL) {
    if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
    if (!has_msg_ext(e->d_name)) continue;
    if (consume_and_move(mdir_user, e->d_name)) processed++;
  }
  closedir(d);
  fprintf(stderr, "(%s) received: %d\n", user, processed);
  return processed;
}
#endif

/* ---------- CLI helpers ---------- */

static const char* basename_only(const char* s) {
  const char* p = s + strlen(s);
  while (p > s) {
#if defined(_WIN32) || defined(__human68k__)
    if (p[-1] == '\\' || p[-1] == '/' || p[-1] == ':') break;
#else
    if (p[-1] == '/' ) break;
#endif
    p--;
  }
  return p;
}

/* Parse message argument from argv, emulating simple double-quote semantics.
 * - If any '"' appears in argv[3..], the message is the text between the
 *   first '"' and the last '"' across those arguments (quotes are removed).
 * - Arguments after the closing quote are not included in the message.
 * - If no '"' is found, argv[3] is used as the message.
 * - next_index (if non-NULL) receives the index of the first argument after
 *   the closing quote (or 4 if no quotes and argv[3] was used).
 */
static void parse_message(char* buf, size_t cap,
                          int argc, char** argv,
                          int* next_index) {
  int i;
  int start = 3;
  int end = 3;
  int found_quote = 0;
  size_t n = 0;

  if (cap == 0) return;
  buf[0] = '\0';
  if (next_index) *next_index = argc;
  if (argc <= 3) return;

  /* find first and last argument that contains a double quote */
  for (i = 3; i < argc; ++i) {
    const char* s = argv[i];
    const char* q = strchr(s, "\""[0]);
    if (q) {
      if (!found_quote) {
        start = i;
        found_quote = 1;
      }
      end = i;
    }
  }

  if (!found_quote) {
    /* no quotes: treat argv[3] as the message */
    strncpy(buf, argv[3], cap - 1);
    buf[cap - 1] = '\0';
    if (next_index) *next_index = (argc > 4) ? 4 : argc;
    return;
  }

  /* build message from argv[start..end], stripping '"' and inserting spaces */
  for (i = start; i <= end; ++i) {
    const char* s = argv[i];
    const char* p = s;
    while (*p) {
      if (*p != '"') {
        if (n + 1 >= cap) { buf[n] = '\0'; goto done; }
        buf[n++] = *p;
      }
      p++;
    }
    if (i < end) {
      if (n + 1 >= cap) { buf[n] = '\0'; goto done; }
      buf[n++] = ' ';
    }
  }
  buf[n] = '\0';

done:
  if (next_index) *next_index = end + 1;
}

static void usage(const char* argv0) {
  const char* base = basename_only(argv0);
  fprintf(stderr,
    "Usage:\n"
    "  %s send all \"message\"\n"
    "  %s send <user> \"message\"\n"
    "  %s recv <user>\n", base, base, base);
}

/* ---------- main ---------- */

int main(int argc, char** argv) {
  const char* root = get_root();
  srand((unsigned)time(NULL) ^ (unsigned)clock());

  if (argc < 3) { usage(argv[0]); return 1; }

  if (strcmp(argv[1], "send") == 0) {
    if (argc < 4) { usage(argv[0]); return 1; }

    /* Parse message in a shell-independent way ("...") */
    char msgbuf[LINELEN];
    int next_index = argc; /* reserved for future options */
    parse_message(msgbuf, sizeof(msgbuf), argc, argv, &next_index);
    (void)next_index; /* currently unused */

    if (strcmp(argv[2], "all") == 0) {
      return (send_to_all(root, msgbuf) == 0) ? 0 : 2;
    } else {
      const char* user = argv[2];
      return (send_to_user(root, user, msgbuf) == 0) ? 0 : 2;
    }
  } else if (strcmp(argv[1], "recv") == 0) {
    const char* user = argv[2];
    return (recv_for_user(root, user) >= 0) ? 0 : 2;
  } else {
    usage(argv[0]); return 1;
  }
}

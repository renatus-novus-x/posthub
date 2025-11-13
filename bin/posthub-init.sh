#!/bin/sh
# ------------------------------------------------------------
# posthub-init.sh
# Initialize posthub directory structure (macOS / Linux)
#
# Creates:
#   posthub/
#     users.txt          (alice, bob)
#     alice/maildir/tmp
#                       /new
#                       /cur
#     bob/maildir/tmp
#                     /new
#                     /cur
# ------------------------------------------------------------

ROOT="posthub"

echo "Initializing posthub directory..."

# Create root directory
mkdir -p "$ROOT"

# Create users.txt with alice and bob
echo "alice" > "$ROOT/users.txt"
echo "bob"  >> "$ROOT/users.txt"

# Helper to create Maildir for one user
create_user() {
  USER="$1"
  echo "Creating Maildir for $USER..."
  mkdir -p "$ROOT/$USER/maildir/tmp"
  mkdir -p "$ROOT/$USER/maildir/new"
  mkdir -p "$ROOT/$USER/maildir/cur"
}

create_user "alice"
create_user "bob"

echo "Done."

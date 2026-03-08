# SSH Commit-Signing Fix (March 8, 2026)

## Problem
Git commit failed with:

```text
error: Public key ~/.ssh/id_ed25519.pub doesn't match private ~/.ssh/id_ed25519?
```

## Root Cause
The file names matched (`id_ed25519` and `id_ed25519.pub`), but the **public key content** did not match the private key.

## What I did
1. Checked Git signing config (`gpg.format`, `commit.gpgsign`, `user.signingkey`).
2. Tested SSH signing directly with `ssh-keygen -Y sign` and reproduced the same mismatch error.
3. Verified mismatch by deriving public key from private key and comparing fingerprints.
4. Backed up the existing public key:
   - `~/.ssh/id_ed25519.pub.bak.<timestamp>`
5. Regenerated the public key from the private key:

```bash
ssh-keygen -y -f ~/.ssh/id_ed25519 > ~/.ssh/id_ed25519.pub
chmod 644 ~/.ssh/id_ed25519.pub
```

6. Re-tested signing with `ssh-keygen -Y sign` and it passed.

## Validation
- New public key fingerprint matches the private key-derived key.
- SSH signing test now succeeds.

## What you should do now
Run your commit again:

```bash
git commit -m "small changes"
```

## Optional (GitHub Verified badge)
If you use GitHub SSH commit signing, add the new `id_ed25519.pub` content to your GitHub **SSH signing keys** settings.

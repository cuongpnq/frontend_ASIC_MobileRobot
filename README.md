# frontend_ASIC_MobileRobot

# GitHub Access Setup

This guide explains how to create a GitHub Personal Access Token (PAT) and configure Git to store credentials for seamless access to private repositories.

---

## 1. Generate a Personal Access Token
1. Log in to your GitHub account.
2. Click your profile picture → **Settings**.
3. Scroll down and select **Developer settings**.
4. Go to **Personal access tokens → Tokens (classic)**.
5. Click **Generate new token**.
6. Select the required scopes:
   - For private repositories, check **repo**.
7. Copy the generated token and store it securely (you won’t be able to view it again later).

---

## 2. Configure Git Credentials
Set Git to remember your credentials:
```bash
git config --global credential.helper store
```

The .git-credentials file stores tokens in plain text. Anyone with access to your home directory can read it.
Clone the repository using **username** and **Personal Access Token**. The credential will be stored. You can access right on the next time.

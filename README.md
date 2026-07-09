# email-notifier

`email-notifier` is a daemon that monitors your email account via IMAP server and sends GUI notifications when new emails are arrived.

The package consists of:
- `email-notifier-daemon`
- `email-notifier` - CLI for managing daemon (setting up, starting/stopping the daemon, getting status, etc)
- `email-notifier-uninstaller`

## Installation

```
chmod +x email-notifier_<version>_install.run 
./email-notifier_<version>_install.run
```

## Usage

```
[Main options]
  --help
  --version

[Commands]
  setup             Setup daemon configuration
    --host            IMAP server hostname
    --port            IMAP server port (1-65535). Optional argument (if not set, default value is 993)
    --login           Email account login
    --mailboxes       Semicolon-separated list of mailboxes to monitor. Optional argument (if not set, default value is INBOX).
    --interval        Mail check interval. Example: 1m30s, 5m, 1h, 3h20m. Optional argument (if not set, default value is 5m)
  start             Activate daemon - start monitoring new mail. Daemon needs to be set up before calling it.
  stop              Deactivate daemon - stop monitoring new mail
  status            Get current status of daemon
  fetch-mailboxes   Fetch available mailboxes at email account.
    --host            IMAP server hostname
    --port            IMAP server port (1-65535). Optional argument (if not set, default value is 993)
    --login           Email account login

[Usage]
  email-notifier --help
  email-notifier --version
  email-notifier setup --host="imap.yandex.ru" --login="nst1911@yandex.ru"
  email-notifier setup --host="imap.yandex.ru" --port=993 --login="nst1911@yandex.ru" --mailboxes="INBOX;SomeOtherMailbox" --interval="3h20m"
  email-notifier start
  email-notifier stop
  email-notifier status
  email-notifier fetch-mailboxes --host="imap.yandex.ru" --login="nst1911@yandex.ru"
  email-notifier fetch-mailboxes --host="imap.yandex.ru" --port=993 --login="nst1911@yandex.ru"

[Uninstallation]
Run 'email-notifier-uninstaller' to uninstall the program.
```

## Third-party libraries

- `Qt 6.2.4`
- `libcurl4` for making IMAP requests
- `libqt6keychain1` for storing passwords in secure keyring 
- `makeself` for avoiding the complexity of distro‑specific packages. Since we use user‑level systemd services, a simple self‑extracting script is more portable, root‑free, and easier to manage.

## Motivation

I use OWA for work email because Linux desktop clients are bloated and overcomplicated. But OWA lacks reliable new-mail notifications — and I kept missing important messages.

I didn't need another email client. I just wanted a simple background service that checks my inbox and shows a native notification when something new arrives. No bloat, no GUI, just a lightweight notifier that does one thing well.

This project is exactly that.

# DONTRUNTS

This is a little project I made to mess with the MBR (Master Boot Record) on Windows. I know Windows internals pretty well, but hadn't actually wiped the MBR before, so I figured I'd give it a go. The whole point here is speed every part of the code is tuned to run as fast as possible.

Usually I write C++, but I wanted to get better at C, so this was a good excuse. Most of my time was spent reading docs and figuring out how to do things the fastest way. If you read the code, you'll see almost every comment is about performance.

## What does this do?

- Pins the process to CPU 0 so the OS scheduler doesn't slow things down.
- Loads NT kernel functions directly for fast, low-level access to disk and system calls.
- Overwrites the MBR (first 512 bytes of the disk) with zeroes, which kills the bootloader instantly.
- Forces a reboot using native NT calls, and falls back to WinAPI if needed.

## How do you use it?

Run it as administrator. Seriously, don't run this on your main system unless you want to lose your bootloader. If your antivirus doesn't catch it, your system will reboot and won't boot up again until you fix the MBR.

I made this for malware analysis and VM testing. If you want to mess with it, go ahead, but don't ask if it's undetectable (it's not). Only use it in a VM or on a test box you don't care about.

## Why did I make this?

Mostly for fun and learning. I wanted to see how fast I could make a destructive tool using direct kernel calls. It's also a good way to practice C and get familiar with NT internals.

## Disclaimer

This is for research and educational purposes only. If you break your computer, lose data, or otherwise mess things up, that's on you. I'm not responsible for any damage caused by running this code.

I tested: https://app.any.run/tasks/528fea20-fcbc-4ff7-8d8a-50ef22e71bee
## License

MIT License

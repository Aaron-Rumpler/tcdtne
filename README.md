# The Character Device That Never Ends

```
> cat /dev/tcdtne
This is the character device that never ends,
It just goes on and on my friends.
Some process started reading it, not knowing what it was,
And it'll continue reading it forever just because...

This is the character device that never ends,
It just goes on and on my friends.
Some process started reading it, not knowing what it was,
And it'll continue reading it forever just because...

This is the character device that never ends,
It just goes on and on my friends.
Some process started reading it, not knowing what it was,
And it'll continue reading it forever just because...
```

## Why?

- Because I'm taking [COSC440 - Advanced Operating Systems](https://www.otago.ac.nz/papers/index.html?papercode=COSC440)
  at the [University of Otago](http://www.otago.ac.nz/), which teaches Linux kernel module development with character
  devices
- Because I thought it would be funny

## Building

First, make sure your system's set up for kernel module development (install `gcc`, `make`, the kernel headers, etc).
Then run `make module` to build the module, and `make load` to load it.

## Usage

It's a read-only character device at `/dev/tcdtne`. Do what you like with it.

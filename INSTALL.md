To compile link, copy or clone libdeflate and zopfli directories, build them

```
$ ln -s /path/to/zopfli zopfli
```

You can also clone using `make zopfli`;

Then build them.

```
$ make -C zopfli
```

and use Makefile

```
$ make -f Makefile
```

This will generate `zipzop` executable.


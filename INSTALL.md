To compile link, copy or clone libdeflate and zopfli directories, build them

```
$ ln -s /path/to/zopfli zopfli
$ ln -s /path/to/libdeflate libdeflate
```

You can also clone using `make libdeflate|zopfli`;

Then build them.

```
$ make -C zopfli
$ make -C libdeflate
```

and use Makefile-1-ld-g

```
$ make -f Makefile-1-ld-g
```

This will generate `zipzop-ld`.


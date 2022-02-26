To compile link or copy libdeflate and zopfli directories, build them

```
$ ln -s /path/to/zopfli zopfli
$ make -C zopfli
$ ln -s /path/to/libdeflate libdeflate
$ make -C libdeflate
```

and use Makefile-1-ld-g

```
$ make -f Makefile-1-ld-g
```

This will generate `zipzop-ld`.


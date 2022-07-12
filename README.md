## fbtest

#### Natively compiling

    make

#### Running

    root@overo:~# ./fbtest -h
    Usage ./fbtest: [-r<red>] [-g<green>] [-b<blue>]
      All colors default to 0xff

Examples

    root@overo:~# ./fbtest
    root@overo:~# ./fbtest -r0x40
    root@overo:~# ./fbtest -r0x40 -g0 -b0
    root@overo:~# ./fbtest -r0x80 -g0 -b0
    root@overo:~# ./fbtest -r0xff -g0 -b0
    root@overo:~# ./fbtest -r0x40 -g0 -b0
    root@overo:~# ./fbtest -r0x40 -g0x40 -b0
    root@overo:~# ./fbtest -r0x40 -g0x40 -b0x30

#### Stop the blinking cursor

Kernel command line parameter

    vt.global_cursor_default=0


#### Wake the framebuffer when it blanks

    echo 0 > /sys/class/graphics/fb0/blank

## LCD 屏幕色彩测试

## 使用RGB565格式

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/ed37d68a-873c-4c76-8bae-be5be5780fe1/Untitled.png)

用法：每个色彩值范围 应在0~255之间

```bash

Usage ./fbtest: [-r<red>] [-g<green>] [-b<blue>]
```

执行以下命令，观察颜色是否正常，有无漏光

```markdown
# 红色
./fbtest -r255 -g0 -b0
# 绿色
./fbtest -r0 -g255 -b0
# 蓝色
./fbtest -r0 -g0 -b255
# 黄色
./fbtest -r255 -g255 -b0
```

# FindingMemo

Framework for dynamically instrumenting the Linux kernel.

## Steps

1. Apply the patch in linux/add-hooks-metadata-section.patch
2. Define hooking functions in *impl.c*
3. Build the kernel module and start instrumenting with `memo`

## Function hook definition

Hooks are defined in the hooks.c module. Below is shown an example of a hook for `load_msg` function defined in ipc/msgutil.c:

`struct msg_msg *load_msg(const void __user *src, size_t len)`

The macro `FM_HOOK_FUNC_DEFINEx` specifies the return type and arguments, where `x` is the number of arguments used by the function.

```
struct msg_msg *msg;

FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src,
                size_t, len)
{
        atomic_set(&curr_hook->mutex, false);
        msg = FM_HOOK_FUNC_PTR(load_msg)(src, len);
        atomic_set(&curr_hook->mutex, true);
        pr_info("fmemo: load_msg(): msg addr: %px\n", msg);
        return msg;
}
```

## sysfs attribute definition

`FM_HOOK_ATTR_DEFINE` macro creates a sysfs attribute file related the hooked function specified, `load_msg` in the example that follows. The file is created in `/sys/module/finder/show/`, with a name that starts with `fm_` and is followed by the name of the function being hooked.

```
FM_HOOK_ATTR_DEFINE(load_msg)
{
        return snprintf(buf, PAGE_SIZE, "%px\n", msg);
}
```

## Instrumentation flow

`memo` is the FindingMemo client. Its options are the follows:

```
test@test:~$ sudo ./memo -h
Usage: memo [OPTION]...
Configuration client to the FindingMemo hooking framework.

Arguments:
  -a, --add             Add hook symbol
  -s, --stop            Stop kernel instrumentation
  -i, --init            Initiate kernel instrumentation
  -h, --help            Display this help and exit
```

1.  Add hook functions

```
test@test:~$ sudo ./memo -a load_msg
Hook added for load_msg.
```

2. Start kernel's dynamic instrumentation

```
test@test:~$ sudo ./memo -i
Linux hooking initiated.
```

3. Stop kernel's dynamic instrumentation

```
test@test:~$ sudo ./memo -s
Linux hooking stopped.
```


## Example of use


A test for the `load_msg` function is found in the test directory.

```
test@test:~$ ./msg-queue-test
+ Message queue creation
+ Send message
msg send: a message at Sat Aug 24 11:57:20 4448289

+ Get message
Message received: a message at Sat Aug 24 11:57:20 4448289
```

Adding the hook and sysfs attribute  defined above for `load_msg` results in the msg address being shown in the kernel log and created sysfs file:

```
[86931.553933] fmemo: load_msg(): msg addr: ffff8880066e9480
```

```
test@test:~$ cat /sys/module/finder/show/fm_load_msg
ffff8880066e9480
```

## Notes

* Enable the Kernel Function Tracer: CONFIG_FUNCTION_TRACER

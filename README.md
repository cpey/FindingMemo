# FindingMemo

Framework for hooking Linux kernel functions. FindingMemo uses ftrace.

## Steps

1. Apply the patch in linux/add-hooks-metadata-section.patch
2. Define hooking functions in *impl.c*
3. Build the kernel module and instrument with `memo`

## Function hooks definition

Hooks are defined in the hooks.c module. Below is shown an example of a hook for `load_msg` function defined in ipc/msgutil.c:

`struct msg_msg *load_msg(const void __user *src, size_t len)`

The macro `FM_HOOK_FUNC_DEFINEx` specifies the return type and arguments, where `x` is the number of arguments used by the function.

```
FM_HOOK_FUNC_DEFINE2(load_msg, struct msg_msg *, const void __user *, src,
                size_t, len)
{
        struct msg_msg *msg;
        atomic_set(&curr_hook->mutex, false);
        msg = FM_HOOK_FUNC_PTR(load_msg)(src, len);
        atomic_set(&curr_hook->mutex, true);
        pr_info("fmemo: load_msg(): msg addr: %px\n", msg);
        return msg;
}
```

## Instrumentation flow

`memo` is the tool used to instrument the hooking framework. Its options are the following:

```
test@test:~$ ./memo -h
Usage: memo [OPTION]...
Instrument the FindingMemo hooking framework.

Arguments:
  -s, --symbol          Symbol to hook
  -r, --stop            Stop the tracer
  -i, --init            Initiate the tracer
  -h, --help            Display this help and exit
```

1.  Add hook functions

```
test@test:~$ sudo ./memo -s load_msg
Hook added for load_msg.
```

2. Start the hooking instrumentation

```
test@test:~$ sudo ./memo -i
Linux hooking initiated.
```

3. Stop the hooking instrumentation

```
test@test:~$ sudo ./memo -r
Linux hooking stopped.
```


## Example of use


A test for the `load_msg` hook is found in the test directory.

```
test@test:~$ ./msg-queue-test
+ Message queue creation
+ Send message
msg send: a message at Sat Aug 24 11:57:20 4448289

+ Get message
Message received: a message at Sat Aug 24 11:57:20 4448289
```

Adding the hook defined above for `load_msg` results in the msg address being shown in the kernel log:

```
[86931.553933] fmemo: load_msg(): msg addr: ffff888005d42a80
```


## Notes

* Enable the Kernel Function Tracer: CONFIG_FUNCTION_TRACER

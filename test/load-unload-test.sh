#! /bin/bash

TEST_NUMBER=${1:-0}
DELAY=0

exec_and_check()
{
    eval $1
    if [[ $? != 0 ]]; then
        echo -e "\n** Test failed **\n"
        exit 1
    fi
    sleep $DELAY
}

test_1()
{
    echo "+ Test 1"
    exec_and_check "sudo /sbin/insmod finder.ko"
    exec_and_check "sudo /sbin/rmmod finder"
}

test_2()
{
    echo "+ Test 2"
    exec_and_check "sudo /sbin/insmod finder.ko"
    exec_and_check "sudo ./memo -a load_msg"
    exec_and_check "sudo /sbin/rmmod finder"
}

test_3()
{
    echo "+ Test 3"
    exec_and_check "sudo /sbin/insmod finder.ko"
    exec_and_check "sudo ./memo -a load_msg"
    exec_and_check "sudo ./memo -i"
    exec_and_check "sudo /sbin/rmmod finder"
}

test_4()
{
    echo "+ Test 4"
    exec_and_check "sudo /sbin/insmod finder.ko"
    exec_and_check "sudo ./memo -a load_msg"
    exec_and_check "sudo ./memo -i"
    exec_and_check "sudo ./memo -s"
    exec_and_check "sudo /sbin/rmmod finder"
}

test_5()
{
    echo "+ Test 5"
    exec_and_check "sudo /sbin/insmod finder.ko"
    exec_and_check "sudo ./memo -a load_msg"
    exec_and_check "sudo ./memo -a free_msg"
    exec_and_check "sudo ./memo -i"
    exec_and_check "sudo ./memo -s"
    exec_and_check "sudo /sbin/rmmod finder"
}

if [[ $TEST_NUMBER -eq 0 ]]; then
    test_1
    test_2
    test_3
    test_4
    test_5
elif [[ $TEST_NUMBER -eq 1 ]]; then
    test_1
elif [[ $TEST_NUMBER -eq 2 ]]; then
    test_2
elif [[ $TEST_NUMBER -eq 3 ]]; then
    test_3
elif [[ $TEST_NUMBER -eq 4 ]]; then
    test_4
elif [[ $TEST_NUMBER -eq 5 ]]; then
    test_5
fi

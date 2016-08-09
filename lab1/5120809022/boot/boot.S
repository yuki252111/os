You cannot sleep 
or call something that might sleep
You cannot refer to current
You cannot allocate memory with GPF_KERNEL (which can sleep), you must use GPF_ATOMIC (which can fail)
You cannot call schedule()
You cannot do a down() semaphore call
However, you can do an up()
You cannot transfer data to/from user space
E.g., copy_to_user(), copy_from_user()

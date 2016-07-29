#HZ-RTOS

1. SVC switch:
	scheduler switch to kernel thread, make SP = MSP, CPU is in privilege mode
	
	scheduler switch to user thread, make SP = PSP, CPU is in unprivilege mode
	
2. data access	
	user thread access the data of kernel, swicth to kernel mode, access the data, then swicth to user mode

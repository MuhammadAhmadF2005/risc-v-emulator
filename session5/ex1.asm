.text
main:
	li a7, 5
	ecall
	mv t0, a0
	ecall
	add a0, a0, t0
	li a7, 1
	ecall
	

	# Exit program
	li a7, 10
	ecall


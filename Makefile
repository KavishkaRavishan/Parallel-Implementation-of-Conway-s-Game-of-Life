.PHONY: all serial openmp mpi cuda hybrid clean

all: serial openmp mpi cuda hybrid

serial:
	$(MAKE) -C serial

openmp:
	$(MAKE) -C openmp

mpi:
	$(MAKE) -C mpi

cuda:
	$(MAKE) -C cuda

hybrid:
	$(MAKE) -C hybrid

clean:
	-$(MAKE) -C serial clean
	-$(MAKE) -C openmp clean
	-$(MAKE) -C mpi clean
	-$(MAKE) -C cuda clean
	-$(MAKE) -C hybrid clean

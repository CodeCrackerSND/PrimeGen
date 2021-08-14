PrimeGen - prime generator based on Lint05 libray.
PrimeGen is a Visual Studio 2008 project.
Generates 2048 bits prime number in 7-12 seconds.
Lint05 libray doesn't like MFC librarie (stdfax.h)
so I had to create this program naively (native c++ - no form).
Lint05 multiplication is 10x faster then naive approach,
but unfortunately 10x++ slower then GMP math library,
so DON'T have to be used when speed is critical,
for testing if number is probable prime or
getting the size in bits of a number (bits count)
I think is just fine.


# some coordinate tests (not unit tests)
import operator as op
from functools import reduce

SIZE = 32

def ID3D(x, y, z, h, w):
	return (x + h * (y + w * z))

def TestID3D():
	for z in range(SIZE):
		for x in range(SIZE):
			for y in range(SIZE):
				print(ID3D(x, y, z, SIZE, SIZE))

def TestBitAnd():
	print(1)

def ncr(n, r):
	if (r > n):
		return 0
	r = min(r, n-r)
	numer = reduce(op.mul, range(n, n-r, -1), 1)
	denom = reduce(op.mul, range(1, r+1), 1)
	return numer / denom

def nCr(n, k):
	if (k > n):
		return 0
	sum = 1;
	for i in range(1, k + 1):
		sum = sum * ((n + 1 - i) / i) # '//' means integer division
	return sum

def Bernstein(n, v, t):
	return nCr(n, v) * pow(t, v) * pow(1 - t, n - v);

def DeCasteljau(t):
	sum = 0;
	for i in range(Degree + 1):
		sum = sum + nCr(Degree, i) * Bernstein(Degree, i, t);
	return sum;

WIDTH = 10
HEIGHT = 11
DEPTH = 12
# "1"D to 3D coord
def ID23D(index):
	k = index // (WIDTH * HEIGHT) 
	j = (index % (WIDTH * HEIGHT)) // WIDTH
	i = index - j * WIDTH - k * WIDTH * HEIGHT
	return int(i),int(j),int(k)

def main():
	#for x in range(-256, 256):
		#print(x & SIZE)
	"""# test n choose k function w/ "confirmed" correct one
	for n in range(0, 5):
		for r in range (0, 5):
			if (ncr(n, r) != nCr(n, r)):
				print(n, r, "incorrect")
	#"""

	"""# test DeCasteljau function for various t
	for t in range(20):
		print(DeCasteljau())
	#"""

	#"""# test flatten function  (ID23D)
	# assume 10*10*10 grid
	for x in range(WIDTH):
		for y in range(HEIGHT):
			for z in range(DEPTH):
				index = x + y * WIDTH + z * WIDTH * HEIGHT
				i, j, k = ID23D(index)
				print(x, y, z)
				print(i, j, k)
				print((x,y,z) == (i,j,k))
				print()
	#"""

if __name__ == '__main__':
	main()
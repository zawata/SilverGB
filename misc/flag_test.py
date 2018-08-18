# A simple script to check the math of the carry and half carry flags

import random

# a unsigned 16bit "cast"
def i(x):
    return x%0xFFFF

# method for check the half carry flag
def check_half_carry(x,y):


def check_half_carry_16(x,y):
    z = i(x)^i(y)
    t = i(x+y)
    return (z^t) & 0x1000 != 0

# method for check the carry flag
def check_carry(x,y):
    z = i(x)^i(y)
    t = i(x+y)
    return (z^t) & 0x100 != 0

def check_carry_16(x,y):
    z = i(x)^i(y)
    t = (x+y)
    return (z^t) & 0x10000 != 0

print()

#check 8-bit first
print("8 bit")
#checking the carry flag
print("carry:", end=" ")
for c in range(100):
    x = random.randint(0,0xFF)
    y = random.randint(0,0xFF)

    t1 = (x+y > 255)
    t2 = check_carry(x,y)
    #print(x,y,t1,t2)
    if t1 != t2:
        print("error", c)
        print(x,y,t1,t2)
        exit(1)
print("success")

#checking the half carry flag
print("half_carry:", end=" ")
for c in range(100):
    x = random.randint(0,0xFF)
    y = random.randint(0,0xFF)

    t1 = ((x&0xf)+(y&0xf) > 0xf)
    t2 = check_half_carry(x,y)
    #print(x,y,t1,t2)
    if t1 != t2:
        print("error", c)
        print(x,y,t1,t2)
        exit(1)
print("success")

#check 16 bit math
print("16 bit")
#check carry flag
print("carry:", end=" ")
for c in range(100):
    x = random.randint(0,0xFFFF)
    y = random.randint(0,0xFFFF)

    t1 = (x+y > 0xFFFF)
    t2 = check_carry_16(x,y)
    #print(x,y,t1,t2)
    if t1 != t2:
        print("error", c)
        print(x,y,t1,t2)
        exit(1)
print("success")

#check the half-carry flag
print("half_carry:", end=" ")
for c in range(100):
    x = random.randint(0,0xFFFF)
    y = random.randint(0,0xFFFF)

    t1 = ((x&0xFFF)+(y&0xFFF) > 0xFFF)
    t2 = check_half_carry_16(x,y)
    #print(x,y,t1,t2)
    if t1 != t2:
        print("error", c)
        print(x,y,t1,t2)
        exit(1)
print("success")


import random as rand;
grasses = [rand.randrange(0, 15) for i in range(10)];
print("int grasses[10] = {", end="");
for i in range(0, 10):
    print(grasses[i], end=", " if i < 9 else "};");

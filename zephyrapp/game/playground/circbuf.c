#include <stdio.h>

int buf[7];
int head = 0;

void reset()
{
	for(int i = 0; i < 7; i++)
		buf[i] = 0;
	head = 0;
}

void push(int x)
{
	buf[head] = x;
	head = (head+1)%7;
}

int get(int i)
{
	i = (head+i)%7;
	return buf[i];
}

int main()
{
	reset();
	int x = 0;

	while(getchar())
	{
		push(x);
		x += 1;

		printf("---\n");
		for(int i = 0; i < 7; i++)
		{
			printf("%d: %d\n", i, get(i));
		}
	}
	return 0;
}

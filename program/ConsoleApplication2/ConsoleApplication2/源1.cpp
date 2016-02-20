#include <iostream>
#include <string>

using namespace std;
char

*faces = "RLFBUD",

*order = "AECGBFDHIJKLMSNTROQP",

*bithash = "TdXhQaRbEFIJUZfijeYV",

*perm = "AIBJTMROCLDKSNQPEKFIMSPRGJHLNTOQAGCEMTNSBFDHORPQ",

pos[20], ori[20], val[20],

TEMP,

*tables[8];

int movecube[20], moveamount[20],

phase = 0,

tablesize[] = { 1,4096,  6561,4096,  256,1536,  13824,576 };

#define SWAP(a,b) TEMP=a;a=b;b=TEMP;

#define CHAROFFSET 65

static string answer = "";

void cycle(char*p, char*a)
{
	SWAP(p[*a - CHAROFFSET], p[a[1] - CHAROFFSET]);
	SWAP(p[*a - CHAROFFSET], p[a[2] - CHAROFFSET]);
	SWAP(p[*a - CHAROFFSET], p[a[3] - CHAROFFSET]);
}

void twist(int i, int a)
{
	i -= CHAROFFSET;
	ori[i] = (ori[i] + a + 1) % val[i];
}

void reset()
{
	for (int i = 0; i < 20; pos[i] = i, ori[i++] = 0);
}

int permtonum(char* p)
{
	int n = 0;
	for (int a = 0; a < 4; a++)
	{

		n *= 4 - a;
		for (int b = a; ++b < 4; )
			if (p[b] < p[a]) n++;
	}
	return n;
}

void numtoperm(char* p, int n, int o)
{
	p += o;
	p[3] = o;
	for (int a = 3; a--;)
	{
		p[a] = n % (4 - a) + o;
		n /= 4 - a;
		for (int b = a; ++b < 4; )
			if (p[b] >= p[a]) p[b]++;
	}
}

int getposition(int t)
{
	int i = -1, n = 0;
	switch (t)
	{
	case 1:
		for (; ++i < 12;) n += ori[i] << i;
		break;
	case 2:
		for (i = 20; --i > 11;) n = n * 3 + ori[i];
		break;
	case 3:
		for (; ++i < 12;) n += (pos[i] & 8) ? (1 << i) : 0;
		break;
	case 4:
		for (; ++i < 8;) n += (pos[i] & 4) ? (1 << i) : 0;
		break;
	case 5:
		int corn[8], j, k, l, corn2[4];
		k = j = 0;
		for (; ++i < 8;)
			if ((l = pos[i + 12] - 12) & 4)
			{
				corn[l] = k++;
				n += 1 << i;
			}
			else corn[j++] = l;
			for (i = 0; i < 4; i++) corn2[i] = corn[4 + corn[i]];
			for (; --i;) corn2[i] ^= corn2[0];
			n = n * 6 + corn2[1] * 2 - 2;
			if (corn2[3] < corn2[2])n++;
			break;
	case 6:
		n = permtonum(pos) * 576 + permtonum(pos + 4) * 24 + permtonum(pos + 12);
		break;
	case 7:
		n = permtonum(pos + 8) * 24 + permtonum(pos + 16);
		break;
	}
	return n;
}

void setposition(int t, int n)
{
	int i = 0, j = 12, k = 0;
	char *corn = "QRSTQRTSQSRTQTRSQSTRQTSR";
	reset();
	switch (t) {

	case 1:
		for (; i < 12; i++, n >>= 1) ori[i] = n & 1;
		break;
	case 2:
		for (i = 12; i < 20; i++, n /= 3) ori[i] = n % 3;
		break;
	case 3:
		for (; i < 12; i++, n >>= 1) pos[i] = 8 * n & 8;
		break;
	case 4:
		for (; i < 8; i++, n >>= 1) pos[i] = 4 * n & 4;
		break;
	case 5:
		corn += n % 6 * 4;
		n /= 6;
		for (; i < 8; i++, n >>= 1)
			pos[i + 12] = n & 1 ? corn[k++] - CHAROFFSET : j++;
		break;
	case 6:
		numtoperm(pos, n % 24, 12); n /= 24;
		numtoperm(pos, n % 24, 4); n /= 24;
		numtoperm(pos, n, 0);
		break;
	case 7:
		numtoperm(pos, n / 24, 8);
		numtoperm(pos, n % 24, 16);
		break;
	}
}

void domove(int m)
{
	char *p = perm + 8 * m, i = 8;

	cycle(pos, p);
	cycle(ori, p);
	cycle(pos, p + 4);
	cycle(ori, p + 4);
	if (m < 4)
		for (; --i>3;) twist(p[i], i & 1);
	if (m < 2)
		for (i = 4; i--;) twist(p[i], 0);
}

void filltable(int ti)
{
	int n = 1, l = 1, tl = tablesize[ti];

	char* tb = tables[ti] = new char[tl];

	memset(tb, 0, tl);
	reset();
	tb[getposition(ti)] = 1;

	while (n)
	{
		n = 0;
		for (int i = 0; i < tl; i++) {
			if (tb[i] == l)
			{
				setposition(ti, i);
				for (int f = 0; f < 6; f++)
				{
					for (int q = 1; q<4; q++)
					{
						domove(f);
						int r = getposition(ti);
						if ((q == 2 || f >= (ti & 6)) && !tb[r])
						{
							tb[r] = l + 1;
							n++;
						}
					}
					domove(f);
				}
			}
		}
		l++;
	}
}

bool searchphase(int movesleft, int movesdone, int lastmove)
{
	if (tables[phase][getposition(phase)] - 1 > movesleft ||
		tables[phase + 1][getposition(phase + 1)] - 1 > movesleft) return false;

	if (!movesleft) return true;

	for (int i = 6; i--;) {
		if (i - lastmove && (i - lastmove + 1 || i | 1))
		{
			movecube[movesdone] = i;
			for (int j = 0; ++j < 4;) {
				domove(i);
				moveamount[movesdone] = j;
				if ((j == 2 || i >= phase) &&
					searchphase(movesleft - 1, movesdone + 1, i)) return true;
			}
			domove(i);
		}
	}
	return false;
}

int check()
{
	for (int i = 0; i < answer.length() - 2; i += 2)
	{
		if (answer[i] == answer[i + 2])
		{
			answer[i + 1] = (answer[i + 1] + answer[i + 3]) % 4 + '0';
			answer.erase(i + 2, 2);
		}
	}
	return 1;
}

int main(string state[])
{
	int f, i = 0, j = 0, k = 0, pc, mor;
	for (; k < 20; k++) val[k] = k < 12 ? 2 : 3;
	for (; j < 8; j++) filltable(j);

	for (; i < 20; i++)
	{
		f = pc = k = mor = 0;
		for (; f < val[i]; f++)
		{
			j = strchr(faces, state[i][f]) - faces;
			if (j > k) { k = j; mor = f; }
			pc += 1 << j;
		}
		for (f = 0; f < 20; f++)
			if (pc == bithash[f] - 64) break;
		pos[order[i] - CHAROFFSET] = f;
		ori[order[i] - CHAROFFSET] = mor%val[i];
	}

	for (; phase < 8; phase += 2)
	{
		for (j = 0; !searchphase(j, 0, 9); j++);
		for (i = 0; i < j; i++)
		{
			answer += "FBRLUD"[movecube[i]];
			answer += (moveamount[i] + '0');
		}

	}
	cout << endl << "answer:" << answer << endl;
	check();
	cout << "checking......\nanswer:" << answer << endl;
	return 1;
	cout << "send command" << endl;

}
monitor nmbb
{
	condition okW, okR;
	queue buf[BUFSIZE];
	waitW = { }; // Python-like dictionary
	waitR = { };
	
	procedure entry write(int n, struct elem *v)
	{
		if (n > BUFSIZE) return;
		
		if (n + buf.Length > BUFSIZE)
		{
			waitW[n] = waitW.get(n, condition()).wait();
		}
		
		for (int i = 0; i < n; i++)
			buf[i + buf.Length] = v[i];
		
		int i = buf.Length;
		while (i > 0 && !waitR[i])
			i--;
		if (i > 0)
			waitR[i].signal();
	}
	
	procedure entry read(int m, struct elem *w)
	{
		if (m > BUFSIZE) return;
		
		if (m > buf.Length)
			okR.wait();
			
		for (int i = 0; i < m; i++)
			w[i] = buf.dequeue();
			
		
		okW.signal();
	}
}

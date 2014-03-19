monitor mvie
{
	condition okP, okG;
	queue buf[M];
	
	bool isOneEmpty()
	{
		for (int i = 0; i < M; i++)
			if (buf[i].isEmpty())
				return true;
				
		return false;
	}
	
	procedure entry put(generic *object)
	{
		for (int i = 0; i < M; i++)
			if (buf[i].length() < MELEM)
				buf[i].enqueue(object[i]);
		
		okG.signal();
	}
	
	procedure entry generic *get(int n)
	{
		if (isOneEmpty())
			okG.wait();
		
		generic output = buf[n].dequeue();
		okP.signal();
		return &output;
	}
}

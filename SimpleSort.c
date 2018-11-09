#include <stdio.h>




int data[]={3,2,4,9,1,5,7,6,8,10};



//冒泡排序
void BubbleSort(int data[])
{
	int i,j;
	int temp=0;
	int count=0;
	for(i=0;i<10;i++)
	{
		for(j=i+1;j<10;j++)
		{
			if(data[i]>data[j])
			{
				temp=data[j];
				data[j]=data[i];
				data[i]=temp;
			}
			count++;
		}	
	}


	printf("\n冒泡排序输出如下：\n");
	for(i=0;i<10;i++)
	{
		printf("%d\t",data[i]);
	}
	printf("\n");
	printf("count=\t%d\n",count);
}

int FindPosition(int data[],int low,int high)
{
	int keyvalue = data[low];
	int template;
	while(low < high)
	{
		while(low < high && keyvalue <= data[high])
		{	
			high--;		
		}
		if(low < high)
		{
			data[low] = data[high];
			low++;
		}		
		while(low < high && keyvalue >= data[low])
		{
			low++;
		}
		if(low < high)
		{
			data[high] = data[low];
			high--;
		}

	}
	data[low] = keyvalue;
	return low;
}


//快速排序
void QuickSort(int data[],int low,int high)
{
	int Position;		//找基准位
	if(low < high)
	{
		Position = FindPosition(data,low,high);
		QuickSort(data,low,Position-1);
		QuickSort(data,Position+1,high);
	}	
}




//插入排序
void InsertSort(int data[])
{
	int i,j;
	int temp;
	for(i=1;i<10;i++)
	{
		temp=data[i];
		for(j=i;j>0 && temp < data[j-1];j--)
		{
			data[j]=data[j-1];
			data[j-1]=temp;
		}
		printf("\n第%d轮：\n",i);
		for(int p=0;p<10;p++)
		{
			printf("%d\t",data[p]);
		}
		printf("\n");
	}
}

//选择排序
void SelectSort(int data[])
{
	int i,j;
	int MaxIndex;
	int temp=0;
	for(i=0;i<10;i++)
	{
		MaxIndex=i;
		for(j=i+1;j<10;j++)
		{
			if(data[j]>data[MaxIndex])
			{
				MaxIndex=j;
			}
		}
		temp=data[i];
		data[i]=data[MaxIndex];
		data[MaxIndex]=temp;
	}
}


void main()
{
//	BubbleSort(data);
	int i;
//	QuickSort(data,0,9);
//	InsertSort(data);
	SelectSort(data);
	printf("排序后的数组输出如下:\n");
	for(i=0;i<10;i++)
	{
		printf("%d\t",data[i]);
	}
	printf("\n");


}











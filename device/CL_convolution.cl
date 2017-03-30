
//图像卷积核函数
__kernel void convolution(	
					__global int* restrict A,
                    __global int* restrict B,
                    __global int* restrict C,
                    int sum,
                    int img_width,
                    int kernel_width)
{
    //获取索引号，这里是二维的，所以可以取两个
    //否则另一个永远是0
    int col = get_global_id(0);
    int row = get_global_id(1);
   
    int stx = (kernel_width - kernel_width%2)/2;
    int sty = stx;
   
    int nx,ny;
    int totalR=0;
    int totalG=0;
    int totalB=0;
    int nid = 0;
 
    totalR=0;totalG=0;totalB=0;
    nid=0;
 
  if(col<=2 || row<=2 || col>=img_width-2 || row>=img_width-2)
  {
     B[row*img_width*3+col*3+0] = 0;
     B[row*img_width*3+col*3+1] = 0;
     B[row*img_width*3+col*3+2] = 0;
     return;
   }
 
   for(ny=row-sty;ny<=row+sty;ny++)
    {
        for(nx=col-stx;nx<=col+stx;nx++)
        {
            totalR += C[nid] * A[ny*img_width*3+nx*3+0];
            totalG += C[nid] * A[ny*img_width*3+nx*3+1];
            totalB += C[nid] * A[ny*img_width*3+nx*3+2]; 
            nid++;
        }
    }
   
    B[row*img_width*3+col*3+0] = min(255,totalR/sum);
    B[row*img_width*3+col*3+1] = min(255,totalG/sum);
    B[row*img_width*3+col*3+2] = min(255,totalB/sum);
}

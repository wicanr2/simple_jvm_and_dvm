/* 浮點矩陣乘法 (DVM 版, double, 邏輯在 main)。 */
class GEMMfDvm {
    public static void main(String[] args) {
        int n = 2;
        double[] a = new double[4];
        double[] b = new double[4];
        double[] c = new double[4];
        a[0]=1.1; a[1]=2.2; a[2]=3.3; a[3]=4.4;
        b[0]=0.5; b[1]=1.5; b[2]=2.5; b[3]=3.5;
        for (int i=0;i<n;i++)
          for (int j=0;j<n;j++){
            double sum=0;
            for(int k=0;k<n;k++)
              sum = sum + a[i*n+k]*b[k*n+j];
            c[i*n+j]=sum;
          }
        System.out.println("GEMM 2x2 (double) : C = A x B");
        System.out.println("c[0] = " + c[0]);
        System.out.println("c[1] = " + c[1]);
        System.out.println("c[2] = " + c[2]);
        System.out.println("c[3] = " + c[3]);
    }
}

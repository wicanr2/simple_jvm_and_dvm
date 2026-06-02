/*
 * GEMM (General Matrix Multiply) C = A x B 的 Dalvik 版本。
 * 邏輯放在 main: 本 Simple Dalvik VM 從名為 "main" 的方法開始執行。
 */
class GEMMDvm {
    public static void main(String[] args) {
        int n = 2;
        int[] a = new int[4];
        int[] b = new int[4];
        int[] c = new int[4];
        a[0]=1; a[1]=2; a[2]=3; a[3]=4;
        b[0]=5; b[1]=6; b[2]=7; b[3]=8;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++)
                    sum = sum + a[i*n+k] * b[k*n+j];
                c[i*n+j] = sum;
            }
        System.out.println("GEMM 2x2 : C = A x B");
        System.out.println("c[0] = " + c[0]);
        System.out.println("c[1] = " + c[1]);
        System.out.println("c[2] = " + c[2]);
        System.out.println("c[3] = " + c[3]);
    }
}

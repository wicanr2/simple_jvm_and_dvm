/* 運算示範 (DVM 版, 邏輯放 main): 取模/迴圈/位元/比較/負數, 用來擴大 Dalvik opcode 覆蓋。 */
class OpsDvm {
    public static void main(String[] args) {
        int x = 48, y = 36;
        while (y != 0) { int t = x % y; x = y; y = t; }
        System.out.println("gcd(48,36) = " + x);
        int f = 1;
        for (int i = 1; i <= 6; i++) f = f * i;
        System.out.println("6! = " + f);
        int a = 240, b = 60;
        System.out.println("240 & 60 = " + (a & b));
        System.out.println("240 | 60 = " + (a | b));
        System.out.println("240 ^ 60 = " + (a ^ b));
        System.out.println("5 << 3 = " + (5 << 3));
        System.out.println("160 >> 2 = " + (160 >> 2));
        int p = 7, q = 12;
        int max = (p > q) ? p : q;
        int min = (p < q) ? p : q;
        System.out.println("max(7,12) = " + max);
        System.out.println("min(7,12) = " + min);
        System.out.println("-7 = " + (-p));
    }
}

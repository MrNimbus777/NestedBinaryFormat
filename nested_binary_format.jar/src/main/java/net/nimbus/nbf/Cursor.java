package net.nimbus.nbf;

public class Cursor {
    private final byte[] buffer;
    private int          offset;

    public Cursor(byte[] buffer){
        this.buffer = buffer;
        this.offset = 0;
    }
    public Cursor(byte[] buffer, int offset) {
        this.buffer = buffer;
        this.offset = offset;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public int getOffset() {
        return offset;
    }

    public void reset(){ offset = 0; }
    public void move(int n) {
        offset += n;
    }

    public byte nextByte(){
        return buffer[offset++];
    }
    public byte[] nextBytes(int n) {
        byte[] buff = new byte[n];
        for(int i = 0; i < n; i++) {
            buff[i] = nextByte();
        }
        return buff;
    }
    public short nextShort(){
        return (short)(
                ((nextByte() & 0xFF) << 8) |
                 (nextByte() & 0xFF)
        );
    }
    public int nextInt() {
        return ((nextByte() & 0xFF) << 24) |
                ((nextByte() & 0xFF) << 16) |
                ((nextByte() & 0xFF) << 8) |
                (nextByte() & 0xFF);
    }
    public long nextLong() {
        return ((long) (nextByte() & 0xFF) << 56) |
                ((long) (nextByte() & 0xFF) << 48) |
                ((long) (nextByte() & 0xFF) << 40) |
                ((long) (nextByte() & 0xFF) << 32) |
                ((long) (nextByte() & 0xFF) << 24) |
                ((long) (nextByte() & 0xFF) << 16) |
                ((long) (nextByte() & 0xFF) << 8) |
                (long) (nextByte() & 0xFF);
    }
    public float nextFloat(){
        return Float.intBitsToFloat(nextInt());
    }
    public double nextDouble(){
        return Double.longBitsToDouble(nextLong());
    }

    public void writeByte(byte value) {
        buffer[offset++] = value;
    }
    public void writeShort(short value) {
        writeByte((byte) (value >>> 8));
        writeByte((byte) value);
    }
    public void writeInt(int value) {
        writeByte((byte) (value >>> 24));
        writeByte((byte) (value >>> 16));
        writeByte((byte) (value >>> 8));
        writeByte((byte) value);
    }
    public void writeLong(long value) {
        writeByte((byte) (value >>> 56));
        writeByte((byte) (value >>> 48));
        writeByte((byte) (value >>> 40));
        writeByte((byte) (value >>> 32));
        writeByte((byte) (value >>> 24));
        writeByte((byte) (value >>> 16));
        writeByte((byte) (value >>> 8));
        writeByte((byte) value);
    }
    public void writeFloat(float value) {
        writeInt(Float.floatToIntBits(value));
    }

    public void writeDouble(double value) {
        writeLong(Double.doubleToLongBits(value));
    }
}

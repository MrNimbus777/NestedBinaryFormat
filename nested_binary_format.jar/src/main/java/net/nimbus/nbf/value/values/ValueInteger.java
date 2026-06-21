package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;

public class ValueInteger implements Value {

    public static final byte INT8 = 5;
    public static final byte INT16 = 6;
    public static final byte INT32 = 7;
    public static final byte INT64 = 8;
    public static final byte UINT8 = 9;
    public static final byte UINT16 = 10;
    public static final byte UINT32 = 11;
    public static final byte UINT64 = 12;

    public long val;

    private Byte b = null;

    public ValueInteger(long val) {
        this.val = val;
    }


    public ValueInteger setType(byte b){
        this.b = b;
        return this;
    }

    public void resetType(){
        b = null;
    }

    protected void deriveType(){
        if(((long)Byte.MIN_VALUE) <= val && val <= ((long)Byte.MAX_VALUE)) b = INT8;
        else if(0 <= val && val <= 0xFFL) b = UINT8;
        else if(((long)Short.MIN_VALUE) <= val && val <= ((long)Short.MAX_VALUE)) b = INT16;
        else if(0 <= val && val <= 0xFFFFL) b = UINT16;
        else if(((long)Integer.MIN_VALUE) <= val && val <= ((long)Integer.MAX_VALUE)) b = INT32;
        else if(0 <= val && val <= 0xFFFFFFFFL) b = UINT32;
        else b = INT64;
    }

    @Override
    public byte getTypeMarker() {
        if(b == null){
            deriveType();
        }
        return b;
    }

    @Override
    public void encode(Cursor cursor) {
        final byte b = getTypeMarker();
        cursor.writeByte(b);
        switch (b) {
            case INT8, UINT8 -> cursor.writeByte((byte) val);
            case INT16, UINT16 -> cursor.writeShort((short) val);
            case INT32, UINT32 -> cursor.writeInt((int) val);
            case INT64, UINT64 -> cursor.writeLong(val);
            default -> throw new IllegalStateException("Unexpected value: " + b);
        }
    }

    @Override
    public int size() {
        byte b = getTypeMarker();
        return switch (b) {
            case INT8, UINT8 -> 2;
            case INT16, UINT16 -> 3;
            case INT32, UINT32 -> 5;
            case INT64, UINT64 -> 9;
            default -> throw new IllegalStateException("Unexpected value: " + b);
        };
    }

    @Override
    public String toString() {
        return String.valueOf(val);
    }
}

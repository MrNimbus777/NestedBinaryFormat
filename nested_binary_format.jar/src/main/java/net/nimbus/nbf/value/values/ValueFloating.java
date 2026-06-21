package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;

public class ValueFloating implements Value {

    public static final byte FLOAT32 = 13;
    public static final byte FLOAT64 = 14;

    public double val;
    private Byte b = null;

    public ValueFloating(double val) {
        this.val = val;
    }

    public ValueFloating setType(byte b){
        this.b = b;
        return this;
    }
    public void resetType(){
        b = null;
    }

    private void deriveType(){
        if(val == ((float) val)) b = FLOAT32;
        else b = FLOAT64;
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
            case FLOAT32 -> cursor.writeFloat((float) val);
            case FLOAT64 -> cursor.writeDouble(val);
            default -> throw new IllegalStateException("Unexpected value: " + b);
        }
    }

    @Override
    public int size() {
        byte b = getTypeMarker();
        return switch (b) {
            case FLOAT32 -> 5;
            case FLOAT64 -> 9;
            default -> throw new IllegalStateException("Unexpected value: " + b);
        };
    }

    @Override
    public String toString() {
        return String.valueOf(val);
    }
}

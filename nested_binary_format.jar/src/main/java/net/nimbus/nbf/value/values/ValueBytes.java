package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;

import java.util.Arrays;

public class ValueBytes implements Value {
    public byte[] val;
    public ValueBytes(byte[] val){
        this.val = val;
    }

    @Override
    public byte getTypeMarker() {
        return 3;
    }

    @Override
    public void encode(Cursor cursor) {
        cursor.writeByte(getTypeMarker());
        cursor.writeInt(val.length);
        for(byte c : val) cursor.writeByte(c);

    }

    @Override
    public int size() {
        return val.length + 5;
    }

    @Override
    public String toString() {
        return Arrays.toString(val);
    }
}

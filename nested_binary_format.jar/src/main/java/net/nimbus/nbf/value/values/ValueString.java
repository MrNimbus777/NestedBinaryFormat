package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;

public class ValueString implements Value {
    public String val;

    public ValueString(String val){
        this.val = val;
    }

    @Override
    public byte getTypeMarker() {
        return 4;
    }

    @Override
    public void encode(Cursor cursor) {
        cursor.writeByte(getTypeMarker());
        cursor.writeInt(val.length());
        for(byte c : val.getBytes()) cursor.writeByte(c);
    }

    @Override
    public int size() {
        return val.length()+5;
    }

    @Override
    public String toString() {
        return "\"" + val + "\"";
    }
}

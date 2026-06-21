package net.nimbus.nbf.value;

import net.nimbus.nbf.Cursor;

public interface Value {
    byte getTypeMarker();
    void encode(Cursor cursor);
    int size();
}

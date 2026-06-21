package net.nimbus.nbf;

import net.nimbus.nbf.value.Value;

public interface Decoder {
    Value decode(Cursor c);
}

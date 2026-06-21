package net.nimbus.nbf;

import net.nimbus.nbf.value.Value;
import net.nimbus.nbf.value.values.*;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;

public class NestedBinaryFormat {
    private NestedBinaryFormat(){}

    public static byte[] encode(final Value val) {
        Cursor cursor = new Cursor(new byte[val.size()]);
        val.encode(cursor);
        return cursor.getBuffer();
    }
    private static Value decode(byte type, Cursor cursor) {
        return DECODERS[type].decode(cursor);
    }
    public static Value decode(byte[] buffer) {
        Cursor cursor = new Cursor(buffer);
        byte type = cursor.nextByte();
        return decode(type, cursor);
    }
    private static Value decode(Cursor cursor) {
        return decode(cursor.nextByte(), cursor);
    }

    private static final Decoder[] DECODERS = {
            c -> null,
            // Node 1
            c -> {
                int size = c.nextShort();
                Map<String, Value> fields = new LinkedHashMap<>(size);
                for(int i = 0; i < size; ++i){
                    String name = new String(
                            c.nextBytes(c.nextShort()), java.nio.charset.StandardCharsets.UTF_8);
                    Value val  = decode(c);
                    fields.put(name, val);
                }
                return new ValueNode(fields);
            },
            // List 2
            c -> {
                int list_size = c.nextShort();
                byte type     = c.nextByte();
                ArrayList<Value> list = new ArrayList<>(list_size);
                for(int i = 0; i < list_size; ++i) {
                    list.add(decode(type, c));
                }
                return new ValueList(list);
            },
            // Raw 3
            c -> new ValueBytes(c.nextBytes(c.nextInt())),
            // String 4
            c -> new ValueString(new String(c.nextBytes(c.nextInt()), java.nio.charset.StandardCharsets.UTF_8)),
            // int8 5
            c -> new ValueInteger(c.nextByte()).setType(ValueInteger.INT8),
            // int16 6
            c -> new ValueInteger(c.nextShort()).setType(ValueInteger.INT16),
            // int32 7
            c -> new ValueInteger(c.nextInt()).setType(ValueInteger.INT32),
            // int64 8
            c -> new ValueInteger(c.nextLong()).setType(ValueInteger.INT64),
            // uint8 9
            c -> new ValueInteger(c.nextByte()).setType(ValueInteger.UINT8),
            // uint16 10
            c -> new ValueInteger(c.nextShort()).setType(ValueInteger.UINT16),
            // uint32 11
            c -> new ValueInteger(c.nextInt()).setType(ValueInteger.UINT32),
            // uint64 12
            c -> new ValueInteger(c.nextLong()).setType(ValueInteger.UINT64),
            // float32 13
            c -> new ValueFloating(c.nextFloat()).setType(ValueFloating.FLOAT32),
            // float64 14
            c -> new ValueFloating(c.nextDouble()).setType(ValueFloating.FLOAT64),
    };

    public static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < data.length; i++) {
            sb.append(String.format("%02X", data[i] & 0xFF));

            if (i < data.length - 1) {
                sb.append(" ");
            }
        }

        return sb.toString();
    }
}


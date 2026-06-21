package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;
import net.nimbus.nbf.value.ValueBuilder;

import java.util.LinkedHashMap;
import java.util.Map;

public class ValueNode implements Value {
    private final Map<String, Value> fields;

    public ValueNode() {
        this.fields = new LinkedHashMap<>();
    }

    public ValueNode(Map<String, Value> fields) {
        this.fields = new LinkedHashMap<>(fields);
    }

    public ValueNode(String key, Value value) {
        this.fields = new LinkedHashMap<>();
        this.fields.put(key, value);
    }

    public ValueNode put(String key, Value value) {
        this.fields.put(key, value);
        return this;
    }

    public ValueNode put(String key, double value) {
        this.fields.put(key, ValueBuilder.of(value));
        return this;
    }

    public ValueNode put(String key, String value) {
        this.fields.put(key, ValueBuilder.of(value));
        return this;
    }

    public ValueNode put(String key, byte[] value) {
        this.fields.put(key, ValueBuilder.of(value));
        return this;
    }

    public ValueNode put(String key, Map<String, Value> fields) {
        return new ValueNode(key, ValueBuilder.of(fields));
    }

    public Value get(String key) {
        return fields.get(key);
    }

    public Value getOrDefault(String key, Value defaultValue) {
        return fields.getOrDefault(key, defaultValue);
    }

    public Value remove(String key) {
        return fields.remove(key);
    }

    @Override
    public byte getTypeMarker() {
        return 1;
    }

    @Override
    public void encode(Cursor cursor) {
        cursor.writeByte(getTypeMarker());
        cursor.writeShort((short) fields.size());
        for(var entry : fields.entrySet()){
            cursor.writeShort((short) entry.getKey().length());
            for(byte b : entry.getKey().getBytes()) cursor.writeByte(b);
            entry.getValue().encode(cursor);
        }
    }

    @Override
    public int size() {
        int size = 3;
        for(var entry : fields.entrySet()){
            size += (entry.getKey().length() + 2 + entry.getValue().size());
        }
        return size;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("{");
        var it = fields.entrySet().iterator();
        if (it.hasNext()) {
            var entry = it.next();
            builder.append(entry.getKey()).append(": ").append(entry.getValue());
            while (it.hasNext()) {
                entry = it.next();
                builder.append(", ").append(entry.getKey()).append(": ").append(entry.getValue());
            }
        }
        return builder.append("}").toString();
    }
}

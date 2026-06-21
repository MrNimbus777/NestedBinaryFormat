package net.nimbus.nbf.value;

import net.nimbus.nbf.value.values.*;

import java.util.Map;

public class ValueBuilder {
    public static ValueInteger  of(long val)                  { return new ValueInteger(val); }
    public static ValueFloating of(double val)                { return new ValueFloating(val); }
    public static ValueString   of(String val)                { return new ValueString(val); }
    public static ValueBytes    of(byte[] val)                { return new ValueBytes(val); }
    public static ValueNode     of(Map<String, Value> fields) { return new ValueNode(fields); }
    public static ValueList     of(Value... vals)             { return new ValueList(vals); }
    public static ValueNode     of(ValueNode node)            { return node; }
    public static ValueList     of(ValueList list)            { return list; }

    public static ValueList of(long val, long... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }
    public static ValueList of(double val, double... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }
    public static ValueList of(String val, String... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }
    public static ValueList of(byte[] val, byte[]... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }
    public static ValueList of(ValueNode val, ValueNode... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }
    public static ValueList of(ValueList val, ValueList... vals) {
        Value[] values = new Value[vals.length+1];
        values[0] = of(val);
        for(int i = 0; i < vals.length; ++i) {
            values[i+1] = of(vals[i]);
        }
        return new ValueList(values);
    }

    public static ValueNode node()                                      { return new ValueNode(); }
    public static ValueNode node(String key, long value)                { return new ValueNode(key, of(value)); }
    public static ValueNode node(String key, double value)              { return new ValueNode(key, of(value)); }
    public static ValueNode node(String key, String value)              { return new ValueNode(key, of(value)); }
    public static ValueNode node(String key, byte[] value)              { return new ValueNode(key, of(value)); }
    public static ValueNode node(String key, Map<String, Value> fields) { return new ValueNode(key, of(fields)); }
    public static ValueNode node(String key, Value value)               { return new ValueNode(key, value); }

}

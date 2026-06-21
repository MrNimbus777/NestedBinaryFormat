package net.nimbus.nbf.value.values;

import net.nimbus.nbf.Cursor;
import net.nimbus.nbf.value.Value;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public class ValueList implements Value {
    private final List<Value> list;
    private byte type;

    public ValueList(Value... vals){
        this.list = new ArrayList<>(Arrays.asList(vals));
        deriveListType();
    }
    public ValueList(Value val, Value... vals) {
        this.list = new ArrayList<>(vals.length + 1);
        list.add(val);
        this.list.addAll(Arrays.asList(vals));
        deriveListType();
    }
    public ValueList(ArrayList<Value> list){
        this.list = list;
        deriveListType();
    }

    public final List<Value> getList() {
        return Collections.unmodifiableList(list);
    }

    public void add(Value element) {
        if(list.getFirst().getClass() == element.getClass()) {
            list.add(element);
        } else throw new IllegalStateException("Values in a list must be of same type");
    }

    public void remove(int index) {
        list.remove(index);
    }

    public void remove(Value value) {
        list.remove(value);
    }

    public int indexOf(Value value) {
        return list.indexOf(value);
    }

    @Override
    public byte getTypeMarker() {
        return 2;
    }

    private byte deriveIntType(long min, long max) {

        if (min >= 0) {
            if (max <= 0xFFL)
                return ValueInteger.UINT8;

            if (max <= 0xFFFFL)
                return ValueInteger.UINT16;

            if (max <= 0xFFFFFFFFL)
                return ValueInteger.UINT32;

            return ValueInteger.UINT64;
        }

        if (min >= Byte.MIN_VALUE &&
                max <= Byte.MAX_VALUE)
            return ValueInteger.INT8;

        if (min >= Short.MIN_VALUE &&
                max <= Short.MAX_VALUE)
            return ValueInteger.INT16;

        if (min >= Integer.MIN_VALUE &&
                max <= Integer.MAX_VALUE)
            return ValueInteger.INT32;

        return ValueInteger.INT64;
    }

    private byte deriveListType(){
        if(list.isEmpty()) {
            this.type = 0;
            return 0;
        }
        var it = list.iterator();
        byte first_type = it.next().getTypeMarker();
        while (it.hasNext()) {
            byte b = it.next().getTypeMarker();
            if(first_type != b) {
                if(5 <= first_type && first_type <= 12) { // All int types
                    try {
                        long biggest_val = ((ValueInteger) list.getFirst()).val;
                        long smallest_val = biggest_val;
                        for(int i = 1; i < list.size(); i++) {
                            long current_val = ((ValueInteger) list.get(i)).val;
                            if(current_val > biggest_val) biggest_val = current_val;
                            if(current_val < smallest_val) smallest_val = current_val;
                        }
                        this.type = deriveIntType(smallest_val, biggest_val);
                        for (Value value : list) {
                            ((ValueInteger) value).setType(this.type);
                        }
                        return this.type;
                    } catch (Exception ex) {
                        throw new IllegalStateException("Values in a list must be of same type");
                    }
                } else if(13 <= first_type && first_type <= 14) { // All floating types
                    try {
                        byte biggest_type = first_type;
                        for(int i = 1; i < list.size(); i++) {
                            byte type = list.get(i).getTypeMarker();
                            if(type > biggest_type) {
                                biggest_type = type;
                            }
                        }
                        for (Value value : list) {
                            ((ValueFloating) value).setType(biggest_type);
                        }
                        this.type = biggest_type;
                        return biggest_type;
                    } catch (IllegalStateException ex) {
                        throw new IllegalStateException("Values in a list must be of same type");
                    }
                }
                throw new IllegalStateException("Values in a list must be of same type");
            }
        }
        this.type = first_type;
        return first_type;
    }

    @Override
    public void encode(Cursor cursor) {
        cursor.writeByte(getTypeMarker());
        cursor.writeShort((short) list.size());
        cursor.writeByte(deriveListType());
        for(var elem : list) {
            cursor.move(-1);
            int tmp_pos = cursor.getOffset();
            byte tmp = cursor.getBuffer()[tmp_pos];
            elem.encode(cursor);
            cursor.getBuffer()[tmp_pos] = tmp;
        }
    }

    @Override
    public int size() {
        deriveListType();
        int size = 4;
        for(var elem : list){
            size += elem.size() - 1;
        }
        return size;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("[");
        var it = list.iterator();
        if(it.hasNext()) {
            var entry = it.next();
            builder.append(entry.toString());
            while (it.hasNext()){
                entry = it.next();
                builder.append(", ").append(entry.toString());
            }
        }
        return builder.append("]").toString();
    }
}

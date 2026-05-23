const NBF_TYPES = Object.freeze({
    EMPTY   : 0,
    NODE    : 1,
    LIST    : 2,
    RAW     : 3,
    STRING  : 4,
    INT8    : 5,
    INT16   : 6,
    INT32   : 7,
    INT64   : 8,
    UINT8   : 9,
    UINT16  : 10,
    UINT32  : 11,
    UINT64  : 12,
    FLOAT32 : 13,
    FLOAT64 : 14
});

function getNumberType(n) {
    if (!Number.isFinite(n)) return NBF_TYPES.FLOAT64;

    if (Number.isInteger(n)) {
        if (n >= 0) {
            if (n <= 0xFF) return NBF_TYPES.UINT8;
            if (n <= 0xFFFF) return NBF_TYPES.UINT16;
            if (n <= 0xFFFFFFFF) return NBF_TYPES.UINT32;
            return NBF_TYPES.UINT64;
        } else {
            if (n >= -0x80) return NBF_TYPES.INT8;
            if (n >= -0x8000) return NBF_TYPES.INT16;
            if (n >= -0x80000000) return NBF_TYPES.INT32;
            return NBF_TYPES.INT64;
        }
    }

    return NBF_TYPES.FLOAT64;
}

function getType(value) {
    if (value === null || value === undefined) return NBF_TYPES.EMPTY;

    switch (typeof value) {
        case "string":
            return NBF_TYPES.STRING;

        case "number":
            return getNumberType(value);

        case "bigint":
            return NBF_TYPES.INT64;

        case "boolean":
            return NBF_TYPES.UINT8;

        case "object":
            if (Array.isArray(value)) return NBF_TYPES.LIST;
            if (value instanceof ArrayBuffer) return NBF_TYPES.RAW;
            return NBF_TYPES.NODE;
    }

    return NBF_TYPES.EMPTY;
}

class NBFStruct {
    constructor(value) {
        this.type = getType(value);
        this.value = null;

        switch (this.type) {
            case NBF_TYPES.EMPTY:
                this.value = null;
                break;

            case NBF_TYPES.STRING:
                this.value = value;
                break;

            case NBF_TYPES.NODE: {
                this.value = {};
                for (const [k, v] of Object.entries(value)) {
                    this.value[k] = new NBFStruct(v);
                }
                break;
            }

            case NBF_TYPES.LIST: {
                if (value.length === 0) {
                    this.itemType = NBF_TYPES.EMPTY;
                    this.value = [];
                    break;
                }

                const firstType = getType(value[0]);

                for (const v of value) {
                    if (getType(v) !== firstType) {
                        throw new TypeError("NBF LIST must contain a single type only");
                    }
                }

                this.itemType = firstType;
                this.value = value.map(v => new NBFStruct(v));
                break;
            }

            case NBF_TYPES.RAW:
                this.value = value;
                break;

            case NBF_TYPES.UINT8:
            case NBF_TYPES.UINT16:
            case NBF_TYPES.UINT32:
            case NBF_TYPES.UINT64:
            case NBF_TYPES.INT8:
            case NBF_TYPES.INT16:
            case NBF_TYPES.INT32:
            case NBF_TYPES.INT64:
            case NBF_TYPES.FLOAT32:
            case NBF_TYPES.FLOAT64:
                this.value = value;
                break;
        }
    }
}


class BufferWriter {
    constructor(size = 1024) {
        this.buffer = new Uint8Array(size);
        this.offset = 0;
        this.view = new DataView(this.buffer.buffer);
    }

    ensure(size) {
        if (this.offset + size <= this.buffer.length)
            return;

        let newSize = this.buffer.length * 2;

        while (newSize < this.offset + size)
            newSize *= 2;

        const next = new Uint8Array(newSize);
        next.set(this.buffer);

        this.buffer = next;
        this.view = new DataView(next.buffer);
    }

    writeUint8(v) {
        this.ensure(1);
        this.view.setUint8(this.offset, v);
        this.offset += 1;
    }

    writeInt8(v) {
        this.ensure(1);
        this.view.setInt8(this.offset, v);
        this.offset += 1;
    }

    writeUint16(v) {
        this.ensure(2);
        this.view.setUint16(this.offset, v, false);
        this.offset += 2;
    }

    writeInt16(v) {
        this.ensure(2);
        this.view.setInt16(this.offset, v, false);
        this.offset += 2;
    }

    writeUint32(v) {
        this.ensure(4);
        this.view.setUint32(this.offset, v, false);
        this.offset += 4;
    }

    writeInt32(v) {
        this.ensure(4);
        this.view.setInt32(this.offset, v, false);
        this.offset += 4;
    }

    writeBigUint64(v) {
        this.ensure(8);
        this.view.setBigUint64(this.offset, BigInt(v), false);
        this.offset += 8;
    }

    writeBigInt64(v) {
        this.ensure(8);
        this.view.setBigInt64(this.offset, BigInt(v), false);
        this.offset += 8;
    }

    writeFloat32(v) {
        this.ensure(4);
        this.view.setFloat32(this.offset, v, false);
        this.offset += 4;
    }

    writeFloat64(v) {
        this.ensure(8);
        this.view.setFloat64(this.offset, v, false);
        this.offset += 8;
    }

    writeBytes(bytes) {
        this.ensure(bytes.length);
        this.buffer.set(bytes, this.offset);
        this.offset += bytes.length;
    }

    finish() {
        return this.buffer.slice(0, this.offset);
    }
}
const textEncoder = new TextEncoder();


class BufferReader {
    constructor(bytes) {
        this.buffer = bytes;
        this.view = new DataView(bytes.buffer);
        this.offset = 0;
    }

    readUint8() {
        return this.view.getUint8(this.offset++);
    }

    readInt8() {
        return this.view.getInt8(this.offset++);
    }

    readUint16() {
        const v = this.view.getUint16(this.offset, false);
        this.offset += 2;
        return v;
    }

    readInt16() {
        const v = this.view.getInt16(this.offset, false);
        this.offset += 2;
        return v;
    }

    readUint32() {
        const v = this.view.getUint32(this.offset, false);
        this.offset += 4;
        return v;
    }

    readInt32() {
        const v = this.view.getInt32(this.offset, false);
        this.offset += 4;
        return v;
    }

    readBigUint64() {
        const v = this.view.getBigUint64(this.offset, false);
        this.offset += 8;
        return v;
    }

    readBigInt64() {
        const v = this.view.getBigInt64(this.offset, false);
        this.offset += 8;
        return v;
    }

    readFloat32() {
        const v = this.view.getFloat32(this.offset, false);
        this.offset += 4;
        return v;
    }

    readFloat64() {
        const v = this.view.getFloat64(this.offset, false);
        this.offset += 8;
        return v;
    }

    readBytes(len) {
        const out = this.buffer.slice(this.offset, this.offset + len);
        this.offset += len;
        return out;
    }
}
const textDecoder = new TextDecoder();


function encodePayload(writer, struct) {

    switch (struct.type) {

        case NBF_TYPES.EMPTY:
            return;

        case NBF_TYPES.STRING: {
            const bytes = textEncoder.encode(struct.value);

            writer.writeUint32(bytes.length);
            writer.writeBytes(bytes);

            return;
        }

        case NBF_TYPES.INT8:
            writer.writeInt8(struct.value);
            return;

        case NBF_TYPES.INT16:
            writer.writeInt16(struct.value);
            return;

        case NBF_TYPES.INT32:
            writer.writeInt32(struct.value);
            return;

        case NBF_TYPES.INT64:
            writer.writeBigInt64(struct.value);
            return;

        case NBF_TYPES.UINT8:
            writer.writeUint8(struct.value);
            return;

        case NBF_TYPES.UINT16:
            writer.writeUint16(struct.value);
            return;

        case NBF_TYPES.UINT32:
            writer.writeUint32(struct.value);
            return;

        case NBF_TYPES.UINT64:
            writer.writeBigUint64(struct.value);
            return;

        case NBF_TYPES.FLOAT32:
            writer.writeFloat32(struct.value);
            return;

        case NBF_TYPES.FLOAT64:
            writer.writeFloat64(struct.value);
            return;

        case NBF_TYPES.RAW: {
            const bytes = new Uint8Array(struct.value);

            writer.writeUint32(bytes.length);
            writer.writeBytes(bytes);

            return;
        }

        case NBF_TYPES.LIST: {

            writer.writeUint16(struct.value.length);
            writer.writeUint8(struct.itemType);

            for (const item of struct.value) {

                encodePayload(writer, item);
            }

            return;
        }

        case NBF_TYPES.NODE: {

            const entries = Object.entries(struct.value);

            writer.writeUint16(entries.length);

            for (const [key, child] of entries) {

                const keyBytes = textEncoder.encode(key);

                writer.writeUint16(keyBytes.length);
                writer.writeBytes(keyBytes);

                encodeStruct(writer, child);
            }

            return;
        }
    }
}

function encodeStruct(writer, struct) {
    writer.writeUint8(struct.type);
    encodePayload(writer, struct);
}

function decodePayload(reader, type) {

    switch (type) {

        case NBF_TYPES.EMPTY:
            return null;

        case NBF_TYPES.STRING: {
            const len = reader.readUint32();
            const bytes = reader.readBytes(len);
            return textDecoder.decode(bytes);
        }

        case NBF_TYPES.INT8:
            return reader.readInt8();

        case NBF_TYPES.INT16:
            return reader.readInt16();

        case NBF_TYPES.INT32:
            return reader.readInt32();

        case NBF_TYPES.INT64:
            return reader.readBigInt64();

        case NBF_TYPES.UINT8:
            return reader.readUint8();

        case NBF_TYPES.UINT16:
            return reader.readUint16();

        case NBF_TYPES.UINT32:
            return reader.readUint32();

        case NBF_TYPES.UINT64:
            return reader.readBigUint64();

        case NBF_TYPES.FLOAT32:
            return reader.readFloat32();

        case NBF_TYPES.FLOAT64:
            return reader.readFloat64();

        case NBF_TYPES.RAW: {
            const len = reader.readUint32();
            return reader.readBytes(len);
        }

        case NBF_TYPES.LIST: {
            const length = reader.readUint16();
            const itemType = reader.readUint8();

            const arr = [];

            for (let i = 0; i < length; i++) {
                arr.push(decodePayload(reader, itemType));
            }

            return arr;
        }

        case NBF_TYPES.NODE: {
            const count = reader.readUint16();
            const obj = {};

            for (let i = 0; i < count; i++) {
                const keyLen = reader.readUint16();
                const keyBytes = reader.readBytes(keyLen);
                const key = textDecoder.decode(keyBytes);

                const valueType = reader.readUint8();
                const value = decodePayload(reader, valueType);

                obj[key] = value;
            }

            return obj;
        }

        default:
            throw new Error("Unknown type: " + type);
    }
}

export const NBF = Object.freeze({
    encode(any) {
        const writer = new BufferWriter();
        encodeStruct(writer, new NBFStruct(any));
        return writer.finish();
    },
    decode(bytes) {
        const reader = new BufferReader(bytes);
        const type = reader.readUint8();
        return decodePayload(reader, type);
    },
});

export function printBytes(bytes) {
    let out = "";

    for (const b of bytes) {
        out += b.toString(16)
            .toUpperCase()
            .padStart(2, "0") + " ";
    }

    console.log(out);
}
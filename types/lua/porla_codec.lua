---@meta porla_codec

---JSON `null`. A light userdata, only comparable by identity with `codec.json.null`.
---@class PoJsonNull

local codec = {}
codec.base64 = {}
codec.hex    = {}
codec.json   = {}

---@type integer
codec.base64.VARIANT_ORIGINAL = nil

---@type integer
codec.base64.VARIANT_ORIGINAL_NO_PADDING = nil

---@type integer
codec.base64.VARIANT_URLSAFE = nil

---@type integer
codec.base64.VARIANT_URLSAFE_NO_PADDING = nil

---Raises `invalid_params` for an unknown variant.
---@param data string
---@param variant? integer # One of the `VARIANT_*` constants, defaults to `VARIANT_ORIGINAL`
---@return string? value # Decoded bytes, or nil on error
---@return PoError? err # `decode_error`, or nil on success
function codec.base64.decode(data, variant) end

---Raises `invalid_params` for an unknown variant.
---@param data string
---@param variant? integer # One of the `VARIANT_*` constants, defaults to `VARIANT_ORIGINAL`
---@return string
function codec.base64.encode(data, variant) end

---@param data string
---@return string? value # Decoded bytes, or nil on error
---@return PoError? err # `decode_error` for an odd length or a non-hex character, or nil on success
function codec.hex.decode(data) end

---@param data string
---@return string # Lowercase hex
function codec.hex.encode(data) end

---@type PoJsonNull
codec.json.null = nil

---Tags a table so it encodes as `[]` when empty. Returns `tbl` itself, or a new empty table when omitted.
---@generic T: table
---@param tbl? T
---@return T
function codec.json.array(tbl) end

---JSON `null` decodes to `codec.json.null`, not nil, and arrays come back tagged as by `codec.json.array`.
---Integers above the signed 64-bit range decode as floats.
---@param data string
---@return any? value # Decoded value, or nil on error
---@return PoError? err # `decode_error` for malformed JSON or nesting deeper than 256 levels, or nil on success
function codec.json.decode(data) end

---A table with only keys 1..n encodes as an array, any other table as an object; an empty table encodes as
---`{}` unless tagged by `codec.json.array`. nil and `codec.json.null` encode as `null`.
---Raises `invalid_params` for sparse arrays, non-string object keys, NaN or infinity, functions, userdata,
---invalid UTF-8 (base64-encode binary data first) or nesting deeper than 256 levels.
---@param data any
---@return string
function codec.json.encode(data) end

return codec

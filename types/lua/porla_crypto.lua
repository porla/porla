---@meta porla_crypto

local crypto = {}

---HMAC-SHA512-256. Raises `invalid_params` unless `key` is 32 bytes.
---@param input string
---@param key string # From `crypto.auth_keygen`
---@return string mac # 32 raw bytes
function crypto.auth(input, key) end

---@return string key # 32 random bytes
function crypto.auth_keygen() end

---Constant-time. Raises `invalid_params` unless `mac` and `key` are 32 bytes each.
---@param mac string # the result of crypto.auth
---@param input string # the input to verify
---@param key string # the key used to generate the auth
---@return boolean
function crypto.auth_verify(mac, input, key) end

---BLAKE2b. Raises `invalid_params` unless `key` is empty or 16-64 bytes.
---@param input string
---@param key string
---@return string hash # 32 raw bytes
---@overload fun(input: string): string
function crypto.hash(input, key) end

---Streaming variant of `crypto.hash`. Raises `invalid_params` unless `key` is empty or 16-64 bytes.
---@param key string # Pass `""` for an unkeyed hash
---@return SoHashState
function crypto.hash_init(key) end

---Constant-time for equal lengths; strings of different lengths compare unequal.
---@param a string
---@param b string
---@return boolean
function crypto.memcmp(a, b) end

---Argon2id with interactive limits, run on a worker pool. The callback never runs inside the call.
---@param input string
---@param callback fun(err: PoError?, hash: string?) # `hash` is a self-describing ASCII string for `crypto.pwhash_verify`; `err` is `internal_error`
function crypto.pwhash(input, callback) end

---Runs on a worker pool. The callback never runs inside the call.
---@param hash string # From `crypto.pwhash`
---@param input string
---@param callback fun(err: PoError?, result: boolean?) # `result` is false for a mismatch or a malformed hash; `err` is `internal_error`
function crypto.pwhash_verify(hash, input, callback) end

---Raises `invalid_params` outside 0-4096.
---@param len integer
---@return string
function crypto.randombytes(len) end

---Raises `invalid_params` unless `upper` is 1 to 2^32-1.
---@param upper integer
---@return integer # Uniform in `[0, upper)`
function crypto.randombytes_uniform(upper) end

---XSalsa20-Poly1305 with a random nonce. Raises `invalid_params` unless `key` is 32 bytes.
---@param message string
---@param key string # From `crypto.secretbox_keygen`
---@return string box # Self-contained, 41 bytes longer than `message`
function crypto.secretbox(message, key) end

---@return string key # 32 random bytes
function crypto.secretbox_keygen() end

---Raises `invalid_params` unless `key` is 32 bytes.
---@param box string
---@param key string
---@return string? message
---@return PoError? err # `decrypt_failed` for a wrong key or a tampered or foreign box
function crypto.secretbox_open(box, key) end

return crypto

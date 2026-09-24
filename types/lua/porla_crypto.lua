---@meta porla_crypto

local crypto = {}

---@param input string
---@param key string
function crypto.auth(input, key) end

---@return string
function crypto.auth_keygen() end

---@param mac string # the result of crypto.auth
---@param input string # the input to verify
---@param key string # the key used to generate the auth
function crypto.auth_verify(mac, input, key) end

---@param input string
function crypto.hash(input) end

---@param input string
---@param key string
function crypto.hash(input, key) end

---@param key string
---@return SoHashState?
function crypto.hash_init(key) end

---@param a string
---@param b string
---@return boolean
function crypto.memcmp(a, b) end

---@param input string
---@param callback fun(err?: string, hash: string)
function crypto.pwhash(input, callback) end

---@param hash string
---@param input string
---@param callback fun(err?: string, result: boolean)
function crypto.pwhash_verify(hash, input, callback) end

---@param len integer
---@return string
function crypto.randombytes(len) end

---@param upper integer
---@return integer
function crypto.randombytes_uniform(upper) end

---@param message string
---@param key string
function crypto.secretbox(message, key) end

---@returns string
function crypto.secretbox_keygen() end

---@param box string
---@param key string
---@return string?
function crypto.secretbox_open(box, key) end

return crypto

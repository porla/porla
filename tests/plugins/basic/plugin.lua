return {
    name    = "basic",
    version = "0.1",

    init = function()
        porla.on("torrent.added", function(th)
            print("handle is valid?", th:is_valid())
            sleep(1)
            print("sleeping")
            local ts = th:status()
            print(ts.name)
        end)
    end,

    destroy = function()
        print("Bye")
    end
}

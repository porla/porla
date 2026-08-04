return {
    name    = "basic",
    version = "0.1",

    init = function()
        porla.on("torrent.added", function(th)
            local status = th:status()
            print("Torrent", status.name, "loaded")
        end)
    end,

    destroy = function()
        print("Bye")
    end
}

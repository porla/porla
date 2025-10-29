plugin = {
    name    = "basic",
    version = "0.1"
}

function porla.init(config)
    print "porla.init"
    print(config.x)
end

function porla.destroy()
    print "porla.destroy"
end

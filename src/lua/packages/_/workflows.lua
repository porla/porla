R"luastring"--(

local log = require "log"
local pql = require "pql"

local function run(actions, index, ctx)
    if index > #actions then
        log.debug "Workflow run completed"
        return
    end

    log.debug(string.format("Running action %d of %d in workflow", index, #actions))

    actions[index](ctx, function(output)
        ctx.steps[index] = output
        run(actions, index + 1, ctx)
    end)
end

local function add(workflow)
    log.info "Adding workflow"

    local pql_filter = nil

    if type(workflow.filter) == "string" then
        pql_filter = pql.parse(workflow.filter)
    end

    workflow.trigger(function(torrent)
        local ctx = {
            steps   = {},
            torrent = torrent
        }

        local should_execute = true

        if type(workflow.filter) == "function" then
            should_execute = workflow.filter(ctx)
        elseif type(workflow.filter) == "string" then
            should_execute = pql_filter:includes(torrent)
        end

        if not should_execute then
            return
        end

        run(workflow.actions, 1, ctx)
    end)
end

return {
    add = add
}
--)luastring"--"

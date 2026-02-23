import '@g-js-api/g.js'

// configures G.js, so it knows how to export to Geometry Dash
// this NEEDS to be ran before anything else GD-related!
await $.exportConfig({
    type: 'live_editor', // you can change this to 'live_editor' if you want to use it using the WSLiveEditor mod, or 'levelstring' if you only want to export the levelstring (make sure to store the result in a variable!)
    options: { info: true } // displays level info when the program finishes running, check https://g-js-api.github.io/G.js/module-index.html#~save_config for a list of options
});

// for a simple example, let's create some moving text
// first, store a group in a variable (this is akin to doing 'Next Free' in GD)
let my_text = unknown_g();

// now, add some text at X 45 Y 45 with the group ID we defined 
// GD uses small-step units internally (3x big step), meaning the block is 15 steps from the origin in-game
'Hello, World!'
    .to_obj()
    .with(obj_props.X, 45)
    .with(obj_props.Y, 45)
    .with(obj_props.GROUPS, my_text)
    .add();

// let's make a loop that moves this text left and right forever
// a 'trigger function' is a system of Geometry Dash triggers
let moveloop = trigger_function(() => {
    let my_context = $.trigger_fn_context(); // stores the ORIGINAL group of the trigger function (it will change later! this is called "context")
    my_text.move(30, 0, 0.5); // moves the text forwards 30 big step units with a 0.5 move time
    my_text.move(-30, 0, 0.5); // afterwards, it moves the text BACK 30 big step units to its original place

    // flashes the background white WITHOUT changing the context of triggers
    ignore_context_change(() => log.runtime.flash());

    // after the two moves, the "context" changes (meaning spawn delays were applied in-between, therefore newer triggers have different group IDs than in the past)
    // so to loop it, you can just call the group of the original context after all operations are finished
    my_context.call();
})

// now finally, we can spawn this loop!
moveloop.call();
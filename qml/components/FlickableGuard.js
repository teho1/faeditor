.pragma library

function isFlickable(item) {
    return item
            && item.interactive !== undefined
            && item.contentX !== undefined
            && item.contentY !== undefined
            && typeof item.cancelFlick === "function"
}

function overflows(fl) {
    return fl.contentWidth > fl.width + 1 || fl.contentHeight > fl.height + 1
}

function lockFrom(item) {
    const locked = []
    let p = item ? item.parent : null
    while (p) {
        if (isFlickable(p)) {
            p.cancelFlick()
            if (p.pointerLocks !== undefined)
                p.pointerLocks += 1
            else
                p.interactive = false
            locked.push(p)
        }
        p = p.parent
    }
    return locked
}

function unlock(locked) {
    if (!locked)
        return
    for (let i = 0; i < locked.length; ++i) {
        const fl = locked[i]
        if (fl.pointerLocks !== undefined)
            fl.pointerLocks = Math.max(0, fl.pointerLocks - 1)
        else
            fl.interactive = overflows(fl)
    }
}

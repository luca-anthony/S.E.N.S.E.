// =========================================================
// DETERMINATION page — background lyrics
// =========================================================
// This is the ONLY file you need to edit to add your lyrics.
// Everything below is placeholder text so you can see the
// pattern in action — replace it with your real lines.
//
// -------------------------------------------------------
// HOW A LINE WORKS
// -------------------------------------------------------
// Each line is one object: { segments: [ ...pieces... ] }
//
// A line is built out of "segments" — little pieces of text
// that get glued back together in order. Each segment is
// either:
//   { text: "some words" }                 <- stays white
//   { text: "some words", color: "#ffe100" } <- colored
//
// So if you want ONE word colored in a line, split that
// line into three segments: "before the word", "the word"
// (with a color), and "after the word". Everything else
// just stays as plain white segments.
//
// -------------------------------------------------------
// WORKED EXAMPLE
// -------------------------------------------------------
// Say your line is:      SHOWED US MERCY!
// ...and you want "MERCY!" to be yellow (#ffe100), rest white:
//
// { segments: [
//     { text: "SHOWED US " },
//     { text: "MERCY!", color: "#ffe100" }
// ] }
//
// Say your line is:      YOU OPENED OUR HEARTS TO MERCY
// ...and "HEARTS" is red, "MERCY" is yellow, rest white:
//
// { segments: [
//     { text: "YOU OPENED OUR " },
//     { text: "HEARTS", color: "#ff2323" },
//     { text: " TO " },
//     { text: "MERCY", color: "#ffe100" }
// ] }
//
// Tip: keep the spaces INSIDE the plain segments (like the
// " TO " above) so the words don't get glued together with
// no space between them.
//
// -------------------------------------------------------
// SOME HANDY COLORS (feel free to use any hex code)
// -------------------------------------------------------
//   yellow      #ffe100
//   red         #ff2323
//   orange      #ff8c00
//   green       #22c55e
//   light blue  #4fc3f7
//   dark blue   #1e3a8a
//   white       #ffffff   (or just leave color out)
//
// =========================================================

const determinationLyrics = [
    { segments: [{ text: "In the end..." }] },
    { segments: [{ text: "It all comes back to SAVING them!" }] },
    { segments: [{ text: "Say they'll forget me, don't really agree-" }] },
    { segments: [{ text: "They're still in there aren't they?" }] },
    { segments: [{ text: "WHO ARE YOU? HUMAN CHILD," }] },
    { segments: [{ text: "JUST LOST SOULS..." }] },
    { segments: [{ text: "NO CONTROL" }] },
    { segments: [{ text: "THOUGH YOU SEEM FAMILIAR SOMEHOW..." }] },
    { segments: [{ text: "THE WAY YOU SHOW CARE..." }] },
    { segments: [
        { text: "MERCY", color: "#ffe100" },
        { text: " IN YOUR PRAYER!" }
    ]},
    { segments: [{ text: "IT'S ALL COMING BACK NOW..." }] },
    { segments: [{ text: "MEMORIES..." }] },
    { segments: [{ text: "FLOODING BACK!" }] },
    { segments: [{ text: "WE COULD NEVER ATTACK-" }] },
    { segments: [
        { text: "YOU OPENED ALL OUR " },
        { text: "HEARTS", color: "#ff2323" },
        { text: " TO " },
        { text: "MERCY", color: "#ffe100" }
    ]},
    { segments: [{ text: "WE'RE RIGHT HERE WITH YOU" }] },
    { segments: [
        { text: "YOU MUST " },
        { text: "CONTINUE!", color: "#ff8c00" }
    ]},
    { segments: [{ text: "OUR FATE LIES WITH YOU!" }] },
    { segments: [
        { text: "SHOWED " },
        { text: "US", color: "#ffe100" },
        { text: " " },
        { text: "MERCY", color: "#22c55e" },
        { text: "!", color: "#ffe100" }
    ]},
    { segments: [
        { text: "YOU", color: "#4fc3f7" },
        { text: " " },
        { text: "BELIEVED", color: "#1e3a8a" },
        { text: " " },
        { text: "IN", color: "#4fc3f7" },
        { text: " " },
        { text: "US", color: "#4fc3f7" },
        { text: "!", color: "#4fc3f7" }
    ]},
    { segments: [
        { text: "STAY ", color: "#1e3a8a" },
        { text: "DETERMINED", color: "#ff2323" },
        { text: " AND WE KNOW YOU'LL WIN", color: "#1e3a8a" }
    ]},
    { segments: [
        { text: "helped us " },
        { text: "smile,", color: "#ffe100" }
    ]},
    { segments: [
        { text: "Now we needen't " },
        { text: "WORRY", color: "#ff8c00" }
    ]},
    { segments: [
        { text: "We're starting to see that ", color: "#ff8c00" },
        { text: "HAPPY", color: "#4fc3f7" },
        { text: " ", color: "#ff8c00" },
        { text: "END!", color: "#4fc3f7" }
    ]},
    { segments: [{ text: "OUR TRUST IN YOU" }] },
    { segments: [{ text: "KNOWS NO BOUNDS CHILD" }] },
    { segments: [{ text: "ON YOUR SIDE TILL THE VERY END AND-" }] },
    { segments: [{ text: "IF YOU HAVE TO STAY FOR A WHILE," }] },
    { segments: [{ text: "AT LEAST YOU HAVE FAMILY AND FRIENDS!" }] },
];
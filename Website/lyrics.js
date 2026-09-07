document.addEventListener("DOMContentLoaded", function () {
    var container = document.getElementById("lyricsBg");

    if (!container || typeof determinationLyrics === "undefined") {
        return; // this page doesn't have a lyrics background — do nothing
    }

    // Build one <div class="lyrics-line"> per lyric line, made of
    // <span> segments so each piece can have its own color.
    determinationLyrics.forEach(function (line) {
        var lineEl = document.createElement("div");
        lineEl.className = "lyrics-line";

        line.segments.forEach(function (segment) {
            var span = document.createElement("span");
            span.textContent = segment.text;
            if (segment.color) {
                span.style.color = segment.color;
            }
            lineEl.appendChild(span);
        });

        container.appendChild(lineEl);
    });

    // Optional credit line underneath everything
    if (typeof determinationLyricsCredit === "string" && determinationLyricsCredit.trim() !== "") {
        var credit = document.createElement("div");
        credit.className = "lyrics-credit";
        credit.textContent = determinationLyricsCredit;
        container.appendChild(credit);
    }

    // Fade each line in as it scrolls near the middle of the
    // screen, and back out as it scrolls away — same idea as
    // the flashing/fading text on the Undertale title screen.
    var lines = container.querySelectorAll(".lyrics-line");

    if ("IntersectionObserver" in window) {
        var observer = new IntersectionObserver(
            function (entries) {
                entries.forEach(function (entry) {
                    entry.target.classList.toggle("in-view", entry.isIntersecting);
                });
            },
            { threshold: 0.5 }
        );

        lines.forEach(function (lineEl) {
            observer.observe(lineEl);
        });
    } else {
        // Very old browsers without IntersectionObserver: just show them.
        lines.forEach(function (lineEl) {
            lineEl.classList.add("in-view");
        });
    }
});

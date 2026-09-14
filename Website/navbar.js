// =========================================================
// S.E.N.S.E. — Navbar behavior
// This is the only JS the navbar needs. All it does is look
// at the current page's filename and mark the matching link
// in the navbar as "active" (so it gets highlighted). This
// runs automatically on every page that includes this file,
// so you never have to edit it when you add new pages.
// =========================================================

document.addEventListener("DOMContentLoaded", function () {
    // e.g. "/Website/pages/determination.html" -> "determination.html"
    // "/Website/index.html" -> "index.html", "/" -> "index.html"
    var path = window.location.pathname;
    var currentFile = path.substring(path.lastIndexOf("/") + 1) || "index.html";

    var links = document.querySelectorAll(".nav-links a");

    links.forEach(function (link) {
        var linkFile = link.getAttribute("href").split("/").pop();

        if (linkFile === currentFile) {
            link.classList.add("active");
        }
    });
});

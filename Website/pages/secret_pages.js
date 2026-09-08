// =========================================================
// SECRET PAGES JAVASCRIPT — Undertale-style sequences
// =========================================================

document.addEventListener('DOMContentLoaded', function() {
    prepSprites();

    const isRefusedPage = document.body.classList.contains('refused-page');
    const isInspirePage = document.body.classList.contains('inspire-page');

    if (isRefusedPage) {
        initRefusedPage();
    } else if (isInspirePage) {
        initInspirePage();
    }
});

// Show the image when it loads, show the CSS fallback when it fails.
// Image + fallback text never appear side by side.
function prepSprites() {
    document.querySelectorAll('.sprite-img').forEach(function(img) {
        var container = img.parentElement;
        function markOk() { if (container) container.classList.add('has-img'); }
        function markMissing() { img.classList.add('img-missing'); }
        if (img.complete) {
            if (img.naturalWidth > 0) { markOk(); }
            else { markMissing(); }
        } else {
            img.addEventListener('load', markOk);
            img.addEventListener('error', markMissing);
        }
    });
}

function sleep(ms) { return new Promise(resolve => setTimeout(resolve, ms)); }

// Play a one-shot sound effect. Spaces in filenames are URL-encoded.
function playSound(path, volume) {
    var audio = new Audio(encodeURI(path));
    audio.volume = (volume === undefined) ? 0.7 : volume;
    var p = audio.play();
    if (p !== undefined) { p.catch(function(e) { console.log('SFX play failed:', e); }); }
    return audio;
}

// Stop a sound after ms milliseconds, fading out over fadeMs so a
// long clip (like the 2s Soul Break) never overstays its moment.
function stopAfter(audio, ms, fadeMs) {
    setTimeout(function() {
        try {
            var steps = 6;
            var stepMs = (fadeMs || 300) / steps;
            var volStep = audio.volume / steps;
            var n = 0;
            var timer = setInterval(function() {
                n++;
                audio.volume = Math.max(0, audio.volume - volStep);
                if (n >= steps) {
                    clearInterval(timer);
                    audio.pause();
                    audio.currentTime = 0;
                }
            }, stepMs);
        } catch (e) { try { audio.pause(); } catch (_) {} }
    }, ms);
}

// How long to wait for an already-playing sound (real duration when
// metadata is available, otherwise the fallback). Never hangs.
function soundWaitMs(audio, fallbackMs) {
    if (audio.duration && isFinite(audio.duration)) {
        return Math.min(audio.duration * 1000 + 300, fallbackMs);
    }
    return fallbackMs;
}

// =========================================================
// BUT_IT_REFUSED.HTML — Choice Screen Logic
// =========================================================

function initRefusedPage() {
    const hisTheme = document.getElementById('hisTheme');
    const saveBtn = document.getElementById('saveBtn');
    const exitBtn = document.getElementById('exitBtn');
    const choiceBox = document.getElementById('choiceBox');
    const flyingLogo = document.getElementById('flyingLogo');
    const flyingLogoImg = document.getElementById('flyingLogoImg');
    const flyingHeart = document.getElementById('flyingHeart');
    const flyingHeartImg = document.getElementById('flyingHeartImg');
    const flashOverlay = document.getElementById('flashOverlay');

    let sequenceRunning = false;

    function startMusic() {
        hisTheme.volume = 0.6;
        const playPromise = hisTheme.play();
        if (playPromise !== undefined) {
            playPromise.catch(e => {
                console.log('His Theme autoplay blocked, waiting for interaction:', e);
                document.addEventListener('click', function startOnClick() {
                    hisTheme.play().catch(err => console.log('His Theme play failed:', err));
                    document.removeEventListener('click', startOnClick);
                }, { once: true });
            });
        }
    }
    startMusic();

    [saveBtn, exitBtn].forEach(btn => {
        btn.addEventListener('mouseenter', () => { if (!sequenceRunning) btn.style.transform = 'scale(1.05)'; });
        btn.addEventListener('mouseleave', () => { if (!sequenceRunning) btn.style.transform = 'scale(1)'; });
    });

    saveBtn.addEventListener('click', function() {
        if (sequenceRunning) return;
        sequenceRunning = true;
        hisTheme.pause();
        hisTheme.currentTime = 0;
        startSaveSequence();
    });

    exitBtn.addEventListener('click', function() {
        if (sequenceRunning) return;
        sequenceRunning = true;
        hisTheme.pause();
        hisTheme.currentTime = 0;
        startExitSequence();
    });

    // SAVE SEQUENCE — heart appears normally, breaks, then the white
    // fade plays ALONGSIDE the Ruins Door sound (not after it finishes),
    // landing on full white right as the door sound ends → inspire.html
    async function startSaveSequence() {
        choiceBox.style.transition = 'opacity 0.3s ease, transform 0.3s ease';
        choiceBox.style.opacity = '0';
        choiceBox.style.transform = 'scale(0.95)';

        await sleep(100);
        flyingHeart.classList.add('active'); // heart fades in in place (0.6s)

        await sleep(700);
        flyingHeart.classList.add('shake'); // shake while it breaks
        if (flyingHeartImg) flyingHeartImg.src = '../assets/heart_broken.png';
        stopAfter(playSound('../assets/Soul Break.mp3', 0.8), 900, 300);

        await sleep(1000);
        flyingHeart.classList.remove('shake');
        // Hold the heart exactly as it is — without this, removing 'shake'
        // falls back to the .active fade-in rule and replays it, making
        // the heart flicker out and back in.
        flyingHeart.style.animation = 'none';
        flyingHeart.style.opacity = '1';

        await playDoorSyncedWithFade(); // door plays; screen whites out along with it
        window.location.href = 'inspire.html';
    }

    // Plays Ruins Door.mp3 and fades the flash overlay to white over the
    // SAME span as the sound, so the screen finishes going white right as
    // the door sound ends — no dead wait after the sound is done.
    function playDoorSyncedWithFade() {
        return new Promise(resolve => {
            var started = false;
            function startFade(durationMs) {
                if (started) return;
                started = true;
                var fadeSec = Math.max(durationMs / 1000, 0.5);
                flashOverlay.style.transition = 'opacity ' + fadeSec + 's ease';
                flashOverlay.classList.add('hold');
                flashOverlay.style.opacity = '1';
                setTimeout(resolve, durationMs + 300); // small buffer once white
            }
            var audio = playSound('../assets/Ruins Door.mp3', 0.7);
            audio.addEventListener('loadedmetadata', function() {
                var ms = (audio.duration && isFinite(audio.duration)) ? audio.duration * 1000 : 8000;
                startFade(Math.min(ms, 14000));
            });
            audio.addEventListener('error', function() { startFade(3000); });
            // Safety net if metadata never fires
            setTimeout(function() { startFade(8000); }, 1200);
        });
    }

    // EXIT SEQUENCE — LOGO flies up, breaks, back to home
    function startExitSequence() {
        choiceBox.style.transition = 'opacity 0.3s ease, transform 0.3s ease';
        choiceBox.style.opacity = '0';
        choiceBox.style.transform = 'scale(0.95)';

        setTimeout(() => { flyingLogo.classList.add('active'); }, 100);

        setTimeout(() => {
            flyingLogo.classList.add('shake');
            if (flyingLogoImg) flyingLogoImg.src = '../assets/heart_broken.png';
            stopAfter(playSound('../assets/Soul Break.mp3', 0.8), 900, 300);
            setTimeout(() => { flashOverlay.classList.add('flash'); }, 800);
            setTimeout(() => { window.location.href = '../index.html'; }, 1800);
        }, 1400);
    }
}

// =========================================================
// INSPIRE.HTML — Save the World Sequence
// Opens WHITE (from the SAVE fade): 1s hold, slow 2.5s melt
// to black, then broken heart → Heal + swap TOGETHER → text.
// =========================================================

function initInspirePage() {
    const whiteFade = document.getElementById('whiteFade');
    const brokenHeart = document.getElementById('brokenHeart');
    const healedHeart = document.getElementById('healedHeart');
    const dialogueBox = document.getElementById('dialogueBox');
    const dialogueText = document.getElementById('dialogueText');
    const dialogueArrow = document.getElementById('dialogueArrow');
    const finalMessage = document.getElementById('finalMessage');
    const finalText = document.getElementById('finalText');
    const finalButtons = document.getElementById('finalButtons');

    const healSound = document.getElementById('healSound');
    const saveTheWorld = document.getElementById('saveTheWorld');

    startInspireSequence();

    async function startInspireSequence() {
        // 1. Already white. Brief beat, then melt to black.
        whiteFade.classList.add('fade-in');
        await sleep(1000);

        whiteFade.classList.remove('fade-in');
        whiteFade.classList.add('fade-out');
        await sleep(2600); // let the fade land fully

        // 2. BROKEN heart fades in on black
        brokenHeart.classList.add('show');
        await sleep(1200);

        // 3. Heal plays AND the heart swaps on the SAME tick
        healSound.volume = 0.7;
        healSound.currentTime = 0;
        healSound.play().catch(function(e) { console.log('Heal play failed:', e); });
        brokenHeart.classList.remove('show');
        healedHeart.classList.add('show');
        await sleep(soundWaitMs(healSound, 2500));

        // 4. Text box types "*But it refused."
        await sleep(600);
        dialogueBox.classList.add('show');
        await typeText(dialogueText, '*But it refused.');
        dialogueArrow.classList.add('show');

        await waitForUserInput();

        // 5. SAVE The World.mp3 + final message
        dialogueBox.classList.remove('show');
        dialogueArrow.classList.remove('show');

        await sleep(300);
        saveTheWorld.volume = 0.7;
        saveTheWorld.play().catch(e => console.log('SAVE The World play failed:', e));

        const finalLines = [
            'Save the world with us. It won\'t be easy',
            'But that\'s what makes it great.',
            'What makes the adventure amazing.',
            'Because it shows that we work to help each other.',
            'Even if you don\'t help with this project.',
            'Write, draw, help, talk, code, work, model, imagine.',
            'SAVE THE WORLD.',
            'PS: Credit to Toby Fox for icons and music',
            'PPS: Credit to my amazing brother for being the inspiration to everything',
            'I Love you.',
            'LOU MADE THIS BABYYYYYY'
        ];

        finalMessage.classList.add('show');
        await typeTextLines(finalText, finalLines, 30);

        await sleep(500);
        finalButtons.classList.add('show');
    }

    function typeText(element, text, speed = 30) {
        return new Promise(resolve => {
            element.textContent = '';
            let i = 0;
            function type() {
                if (i < text.length) {
                    element.textContent += text.charAt(i);
                    i++;
                    setTimeout(type, speed);
                } else { resolve(); }
            }
            type();
        });
    }

    function typeTextLines(element, lines, speed = 30) {
        return new Promise(async resolve => {
            element.innerHTML = '';
            for (let lineIdx = 0; lineIdx < lines.length; lineIdx++) {
                const lineDiv = document.createElement('div');
                lineDiv.style.marginBottom = lineIdx === lines.length - 1 ? '0' : '12px';
                element.appendChild(lineDiv);
                await typeText(lineDiv, lines[lineIdx], speed);
                await sleep(200);
            }
            resolve();
        });
    }

    function waitForUserInput() {
        return new Promise(resolve => {
            function handler() {
                document.removeEventListener('click', handler);
                document.removeEventListener('keydown', handler);
                resolve();
            }
            document.addEventListener('click', handler);
            document.addEventListener('keydown', handler);
        });
    }
}
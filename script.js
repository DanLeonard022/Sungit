document.addEventListener("DOMContentLoaded", function () {
    const container = document.querySelector(".container");

    container.addEventListener("click", function () {
        this.classList.toggle("open"); // Toggle open class on click
    });
});

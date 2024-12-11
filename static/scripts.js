document.getElementById('checkSimilarity').addEventListener('click', async () => {
    const text1 = document.getElementById('text1').value;
    const text2 = document.getElementById('text2').value;

    try {
        const response = await fetch('/check', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ text1, text2 })
        });

        const data = await response.json();

        if (data.error) {
            alert(`Error: ${data.error}`);
        } else {
            const similarity = parseFloat(data.similarity);
            document.getElementById('similarityScore').textContent = `${similarity}%`;
            document.getElementById('progress').style.width = `${similarity}%`;
        }
    } catch (err) {
        alert('Error connecting to the server.');
    }
});

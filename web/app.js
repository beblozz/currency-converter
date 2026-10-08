const API_URL = "https://open.er-api.com/v6/latest/USD";
const CACHE_KEY = "currency_cache";
const CACHE_LIFETIME = 12 * 60 * 60 * 1000;

const e = React.createElement;

function loadCache() {
    try {
        const text = localStorage.getItem(CACHE_KEY);
        if (!text) {
            return null;
        }
        const data = JSON.parse(text);
        if (!data.values || !data.saved) {
            return null;
        }
        return data;
    } catch (err) {
        return null;
    }
}

function saveCache(data) {
    try {
        localStorage.setItem(CACHE_KEY, JSON.stringify(data));
    } catch (err) {
        console.log("could not save cache");
    }
}

function formatTime(ms) {
    const date = new Date(ms);
    const two = (n) => String(n).padStart(2, "0");
    return date.getFullYear() + "-" + two(date.getMonth() + 1) + "-" + two(date.getDate()) +
        " " + two(date.getHours()) + ":" + two(date.getMinutes());
}

function App() {
    const [rates, setRates] = React.useState(null);
    const [amount, setAmount] = React.useState("100");
    const [from, setFrom] = React.useState("EUR");
    const [to, setTo] = React.useState("USD");
    const [offline, setOffline] = React.useState(false);
    const [loading, setLoading] = React.useState(false);
    const [error, setError] = React.useState("");

    function download(cached) {
        setLoading(true);

        fetch(API_URL)
            .then((response) => {
                if (!response.ok) {
                    throw new Error("server returned code " + response.status);
                }
                return response.json();
            })
            .then((data) => {
                if (data.result !== "success" || !data.rates) {
                    throw new Error("server did not return rates");
                }

                const fresh = {
                    base: data.base_code,
                    updated: data.time_last_update_unix * 1000,
                    saved: Date.now(),
                    values: data.rates
                };

                saveCache(fresh);
                setRates(fresh);
                setOffline(false);
                setError("");
            })
            .catch(() => {
                if (cached) {
                    setRates(cached);
                    setOffline(true);
                    setError("");
                } else {
                    setError("Could not download rates and there is no saved cache. Check your internet connection.");
                }
            })
            .finally(() => {
                setLoading(false);
            });
    }

    React.useEffect(() => {
        const cached = loadCache();

        if (cached && Date.now() - cached.saved < CACHE_LIFETIME) {
            setRates(cached);
            return;
        }

        download(cached);
    }, []);

    function swap() {
        setFrom(to);
        setTo(from);
    }

    let resultBlock = null;

    if (error) {
        resultBlock = e("div", { className: "error" }, error);
    } else if (!rates) {
        resultBlock = e("div", { className: "result" },
            e("div", { className: "result-rate" }, "Loading rates...")
        );
    } else {
        const number = Number(amount);

        if (amount.trim() === "" || !isFinite(number) || number < 0) {
            resultBlock = e("div", { className: "error" }, "Enter a correct amount");
        } else {
            const result = number / rates.values[from] * rates.values[to];
            const one = 1 / rates.values[from] * rates.values[to];

            resultBlock = e("div", { className: "result" },
                e("div", { className: "result-main" },
                    number.toFixed(2) + " " + from + " = " + result.toFixed(2) + " " + to
                ),
                e("div", { className: "result-rate" },
                    "1 " + from + " = " + one.toFixed(4) + " " + to
                )
            );
        }
    }

    let codes = [];
    if (rates) {
        codes = Object.keys(rates.values).sort();
    }

    const options = codes.map((code) => e("option", { key: code, value: code }, code));

    return e("div", { className: "card" },
        e("h1", { className: "title" }, "Currency Converter"),
        e("p", { className: "subtitle" }, "Works offline with saved rates"),

        e("label", { className: "label", htmlFor: "amount" }, "Amount"),
        e("input", {
            id: "amount",
            className: "input",
            type: "number",
            min: "0",
            value: amount,
            onChange: (event) => setAmount(event.target.value)
        }),

        e("div", { className: "row" },
            e("div", { className: "field" },
                e("label", { className: "label", htmlFor: "from" }, "From"),
                e("select", {
                    id: "from",
                    className: "select",
                    value: from,
                    onChange: (event) => setFrom(event.target.value)
                }, options)
            ),
            e("button", { className: "swap", onClick: swap, title: "Swap currencies" }, "⇄"),
            e("div", { className: "field" },
                e("label", { className: "label", htmlFor: "to" }, "To"),
                e("select", {
                    id: "to",
                    className: "select",
                    value: to,
                    onChange: (event) => setTo(event.target.value)
                }, options)
            )
        ),

        resultBlock,

        e("div", { className: "footer" },
            e("div", null,
                rates ? "Rates updated: " + formatTime(rates.updated) : "No rates yet",
                offline ? e("span", { className: "badge" }, "offline") : null
            ),
            e("button", {
                className: "update",
                disabled: loading,
                onClick: () => download(rates)
            }, loading ? "Updating..." : "Update rates")
        )
    );
}

ReactDOM.createRoot(document.getElementById("root")).render(e(App));

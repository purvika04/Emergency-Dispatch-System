// ===============================
// 🌆 Bangalore Map Node Grid (0–19)
// ===============================
const nodeCoords = {
    0:[12.9716,77.5946], 1:[12.9716,77.6000], 2:[12.9716,77.6054], 3:[12.9716,77.6108], 4:[12.9716,77.6162],
    5:[12.9678,77.5946], 6:[12.9678,77.6000], 7:[12.9678,77.6054], 8:[12.9678,77.6108], 9:[12.9678,77.6162],
    10:[12.9640,77.5946], 11:[12.9640,77.6000], 12:[12.9640,77.6054], 13:[12.9640,77.6108], 14:[12.9640,77.6162],
    15:[12.9602,77.5946], 16:[12.9602,77.6000], 17:[12.9602,77.6054], 18:[12.9602,77.6108], 19:[12.9602,77.6162]
    };
    
    let map;
    let unitMarkers = {};
    
    
    // ===============================
    // 🗺 Map Init
    // ===============================
    function initMap() {
        map = L.map('map').setView([12.968, 77.606], 14);
    
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '© OpenStreetMap'
        }).addTo(map);
    
        // draw node markers with index labels
        for (let id in nodeCoords) {
            L.circleMarker(nodeCoords[id], { radius: 6 })
                .addTo(map)
                .bindTooltip("Node " + id, {permanent:true, direction:"top"});
        }
    }
    
    
    // ===============================
    // 🚨 Emergency Reporting
    // ===============================
    function submitEmergency() {
        const sev = document.getElementById('sev').value;
        const loc = document.getElementById('loc').value;
        const typeSelect = document.getElementById('type');
    
        const payload = {
            id: Math.floor(Date.now()/1000),
            severity: parseInt(sev),
            location: parseInt(loc),
            type: typeSelect.options[typeSelect.selectedIndex].text
        };
    
        fetch('/report',{
            method:'POST',
            headers:{'Content-Type':'application/json'},
            body: JSON.stringify(payload)
        }).then(()=>refreshStatus());
    }
    
    
    // ===============================
    function dispatchNext() {
        fetch('/dispatch',{method:'POST'}).then(()=>refreshStatus());
    }
    
    
    // ===============================
    function manualComplete() {
        const unitId = document.getElementById('complete-unit').value;
    
        fetch('/complete',{
            method:'POST',
            headers:{'Content-Type':'application/json'},
            body: JSON.stringify({unitId})
        }).then(()=>refreshStatus());
    }
    
    
    // ===============================
    // 🔄 STATUS + UI + MAP
    // ===============================
    function refreshStatus() {
    fetch('/status')
    .then(r=>r.json())
    .then(data=>{
    
        const container = document.getElementById('units-container');
        const queueCount = document.getElementById('queue-count');
    
        if(queueCount) queueCount.innerText = data.heapSize ?? 0;
        if(!container) return;
    
        container.innerHTML = "";
    
        if(!data.units) return;
    
        // ---------- Cards ----------
        data.units.forEach(unit=>{
            const busy = unit.status==="BUSY";
    
            container.innerHTML += `
            <div class="col">
              <div class="card p-2 border-start border-4 ${busy?'border-danger bg-danger-subtle':'border-success bg-success-subtle'}">
                <h6>Unit ${unit.id} (${unit.type})</h6>
                <p class="small mb-0">Location: ${unit.loc}</p>
                <p class="small mb-0">
                  Status:
                  <b class="${busy?'text-danger':'text-success'}">${unit.status}</b>
                </p>
                <p class="small mb-1">Active ID: ${unit.emergency===-1?'None':unit.emergency}</p>
                <div class="bg-white text-dark rounded p-1 small">
                  <b>Path:</b> ${unit.path || 'Stationary'}
                </div>
              </div>
            </div>`;
        });
    
    
        // ---------- Map Unit Labels ----------
        data.units.forEach(unit=>{
            const coord = nodeCoords[unit.loc];
            if(!coord || !map) return;
    
            const busy = unit.status==="BUSY";
    
            const icon = L.divIcon({
                className: busy ? "unit-label-busy" : "unit-label-free",
                html: `
                    ${unit.type.includes("Fire") ? "🚒" : "🚑"} U${unit.id}
                    <div style="font-size:10px;">Node ${unit.loc}</div>
                `,
                iconSize:[70,34]
            });
    
            if(!unitMarkers[unit.id]){
                unitMarkers[unit.id] = L.marker(coord,{icon}).addTo(map);
            } else {
                unitMarkers[unit.id].setLatLng(coord);
                unitMarkers[unit.id].setIcon(icon);
            }
        });
    
    });
    }
    
    
    // ===============================
    function resetSystem(){
        if(confirm("Reset simulation?")){
            fetch('/reset',{method:'POST'}).then(()=>refreshStatus());
        }
    }
    
    
    // ===============================
    window.onload = ()=>{
        initMap();
        refreshStatus();
    };
    
    setInterval(refreshStatus,2000);
    
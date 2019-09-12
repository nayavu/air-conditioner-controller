import { Component, OnInit } from '@angular/core';
import { HttpClient } from "@angular/common/http";
import { environment } from "../../environments/environment";

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html'
})
export class ConfigComponent implements OnInit {

  public config: Config;
  public status = '';

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/config').subscribe(
        (data: Config) => {
          this.config = data;
        },
        error => {
          console.warn("No configuration found, assuming it's a fresh installation");
          this.config = {
            deviceName: 'remote',
            wifiSsid: '',
            wifiPass: '',
            mqttHost: '',
            mqttPort: 1883,
            mqttUser: '',
            mqttPass: '',
          };
        })
  }

  save() {
    this.status = 'saving';
    this.http.post(environment.backend + '/config', this.config).subscribe(
        (data: Config) => {
          this.status = 'saved';
        },
        error => {
          this.status = 'error';
        })
  }
}

export interface Config {
  deviceName: string;
  wifiSsid: string;
  wifiPass: string;
  mqttHost: string;
  mqttPort: number;
  mqttUser: string;
  mqttPass: string;
}

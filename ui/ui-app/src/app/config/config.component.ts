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
  public msg = '';

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/config').subscribe(
        (data: Config) => {
          this.config = data;
        },
        error => {

          if (error.status == 404) {
            this.msg = 'No persisted configuration found';
          } else if (error.status == 500) {
            this.msg = 'Configuration file corrupted';
          } else {
            this.msg = 'Unknown error with configuration';
          }

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
    this.msg = '';

    this.status = 'saving';
    this.http.post(environment.backend + '/config', this.config).subscribe(
        (data: Config) => {
          this.status = 'saved';
        },
        error => {
          this.status = 'error';
        })
  }

  reboot(configMode = false) {
    this.http.post(environment.backend + '/reboot' + (configMode ? '?config=true' : ''), null).subscribe(
        (data: Config) => {
          this.status = 'rebooting';
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

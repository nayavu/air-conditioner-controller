import { Component, OnInit } from '@angular/core';
import { HttpClient } from "@angular/common/http";
import { environment } from "../../environments/environment";
import { Config } from "../config/config.component";

@Component({
  selector: 'app-devices',
  templateUrl: './devices.component.html'
})
export class DevicesComponent implements OnInit {

  public aircond: AirCond;
  public status = '';

  public modeList = [
    { name: 'Auto', value: 'auto' },
    { name: 'Cool', value: 'cool' },
    { name: 'Heat', value: 'heat' },
    { name: 'Dry',  value: 'dry' }
  ];

  public fanList = [
    { name: 'Auto', value: 0 },
    { name: '1 (min)', value: 1 },
    { name: '2', value: 2 },
    { name: '3', value: 3 },
    { name: '4', value: 4 },
    { name: '5 (max)', value: 5 }
  ];

  public swingList = [
    { name: 'Auto', value: 0 },
    { name: '1 (horizontal)', value: 1 },
    { name: '2', value: 2 },
    { name: '3', value: 3 },
    { name: '4', value: 4 },
    { name: '5 (vertical)', value: 5 }
  ];

  public profileList = [
    { name: 'Normal', value: 0 },
    { name: 'Powerful', value: 1 },
    { name: 'Quiet', value: 2 }
  ];

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/devices/aircond').subscribe((data: AirCond) => {
      this.aircond = data;
    })
  }

  save() {
    this.status = 'saving';
    this.http.post(environment.backend + '/devices/aircond', this.aircond).subscribe(
        (data: Config) => {
          this.status = 'saved';
        },
        error => {
          this.status = 'error';
        });
  }
}

export interface AirCond {
  power: boolean,
  t: number,
  mode: 'auto' | 'heat' | 'cool' | 'dry',
  fan: 'auto' | '1' | '2' | '3' | '4' | '5',
  swing: 'auto' | '1' | '2' | '3' | '4' | '5',
  profile: 'normal' | 'powerful' | 'quiet'
}

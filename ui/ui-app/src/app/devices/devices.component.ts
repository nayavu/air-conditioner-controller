import { Component, OnInit } from '@angular/core';
import { HttpClient } from "@angular/common/http";
import { environment } from "../../environments/environment";

@Component({
  selector: 'app-devices',
  templateUrl: './devices.component.html',
  styleUrls: ['./devices.component.css']
})
export class DevicesComponent implements OnInit {

  public aircond: AirCond;
  public saving = false;

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/devices/aircond').subscribe((data: AirCond) => {
      this.aircond = data;
    })
  }

  save() {
    this.saving = true;
    this.http.post(environment.backend + '/devices/aircond', this.aircond).subscribe(() => {
      this.saving = false;
    })
  }
}

export interface AirCond {
  on: boolean,
  t: number,
  mode: 'auto' | 'heat' | 'cool' | 'dry',
  fan: 'auto' | '1' | '2' | '3' | '4' | '5',
  swing: 'auto' | '1' | '2' | '3' | '4' | '5',
  profile: 'normal' | 'quiet' | 'boost'
}
